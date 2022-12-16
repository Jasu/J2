#include "cli/cli.hpp"
#include "debug/timing.hpp"
#include "lisp/lisp_fwd.hpp"
#include "lisp/assembly/functions/trap_handler.hpp"
#include "lisp/compilation/source_compilation.hpp"
#include "lisp/sources/source.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/context.hpp"
#include "lisp/reader/interactive_reader.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/assembly/disassembly/disassembler.hpp"
#include "lisp/assembly/amd64/abi/ia64_target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "containers/trivial_array.hpp"

namespace j::commands {
  namespace {
    namespace l = j::lisp;
    namespace a = j::lisp::assembly;
    namespace str = j::strings;

    void dump_fn(str::formatted_sink & stdout,
                 const a::disassembly::disassembler * J_NOT_NULL dis,
                 str::const_string_view name,
                 const l::functions::func_info * J_NOT_NULL fn_info,
                 l::functions::calling_convention cc)
    {
      // TODO allocation not set for closures, fix that so they are dumped.
      if (!fn_info->cc_info[cc] || !fn_info->cc_info[cc].allocation) {
        return;
      }
      stdout.write("{} @ 0x{:016X}:\n\n", name, fn_info->cc_info[cc].fn_ptr);
      dis->disassemble_buffer(
        fn_info->cc_info[cc].allocation.code_region(),
        fn_info->cc_info[cc].asm_info,
        [&stdout](const l::assembly::disassembly::instruction & inst) {
          stdout.write("  {}\n", inst);
        });
    }

    void dump_cir(mem::shared_ptr<str::formatted_sink> stdout,
                  bool no_debug_print,
                  bool no_air_dumps,
                  bool no_cir_dumps,
                  str::const_string_view air_dump_path_pattern,
                  str::const_string_view cir_dump_path_pattern,
                  trivial_array<files::path> include_paths,
                  files::path file,
                  trivial_array<strings::const_string_view> eval,
                  trivial_array<strings::const_string_view> load,
                  str::const_string_view input) {
      J_ASSERT_NOT_NULL(stdout);
      l::env::environment env;
      auto guard = l::env::env_context.enter(&env);
      auto tgt = l::assembly::target::get_target(l::assembly::target_name::amd64_ia64);
      auto asmctx = l::assembly::asm_context.enter(tgt);
      l::env::compilation_options opts(tgt,
                                       include_paths
                                       ? static_cast<trivial_array<files::path> &&>(include_paths)
                                       : trivial_array<files::path>(containers::move, { files::path(".") }),
                                       true);
      l::assembly::install_trap_handler();
      if (no_debug_print) {
        opts.air_options.pass_defaults.print_debug = false;
        opts.cir_options.pass_defaults.print_debug = false;
      }
      if (no_air_dumps) {
        opts.air_options.pass_defaults.dump_graph_after = false;
        opts.air_options.pass_defaults.dump_debug_files = false;
        opts.air_options.dump_initial_graph = false;
      }
      if (no_cir_dumps) {
        opts.cir_options.pass_defaults.dump_graph_after = false;
        opts.cir_options.pass_defaults.dump_debug_files = false;
        opts.cir_options.dump_initial_graph = false;
      }
      if (air_dump_path_pattern) {
        opts.air_options.pass_defaults.dump_file_pattern = air_dump_path_pattern;
      }
      if (cir_dump_path_pattern) {
        opts.cir_options.pass_defaults.dump_file_pattern = cir_dump_path_pattern;
      }
      l::env::env_compiler * envc = env.get_env_compiler(&opts);

      auto dis = tgt->disassembler();

      for (auto & pkg : load) {
        stdout->write("Loading package {#bold}{}{/}\n", pkg);
        auto loaded = envc->load_pkg(pkg);
        auto guard2 = l::env::env_context.enter(loaded);
        if (!loaded) {
          stdout->write("{#bright_red,bold}Loading {} failed.{/}\n", pkg);
        } else {
          if (loaded->load_symbol && loaded->load_symbol->value_info.is_function()) {
            dump_fn(*stdout, dis, ":load", loaded->load_symbol->value_info.func_info, l::functions::calling_convention::abi_call);
          }
        }
      }

      l::packages::pkg * input_pkg = nullptr;
      if (input) {
        input_pkg = envc->compile_pkg("input", envc->source_manager.add_static_source("input", input));
      } else if (file) {
        input_pkg = envc->compile_pkg("input", envc->source_manager.add_file_source(file));
      } else {
        input_pkg = envc->create_pkg("input");
      }

      auto guard2 = l::env::env_context.enter(input_pkg);
      if (input_pkg->load_symbol && input_pkg->load_symbol->value_info.is_function()) {
        dump_fn(*stdout, dis, ":load", input_pkg->load_symbol->value_info.func_info, l::functions::calling_convention::abi_call);
      }

      input_pkg->load();

      for (auto & p : input_pkg->symbol_table.map) {
        // Don't use p.second - it might contain a yet uninitialized symbol.
        l::symbols::symbol * s = envc->try_get_symbol(p.first);
        if (!s->value_info.is_function() || s == input_pkg->load_symbol) {
          continue;
        }

        J_ASSUME_NOT_NULL(s->value_info.func_info);
        auto fn = s->value_info.func_info;
        dump_fn(*stdout, dis, "Fn", fn, l::functions::calling_convention::abi_call);
        dump_fn(*stdout, dis, "Fn", fn, l::functions::calling_convention::full_call);
        for (auto & clos : fn->closures) {
          dump_fn(*stdout, dis, "Closure", clos.sym->value_info.func_info, l::functions::calling_convention::full_call);
        }

        try {
        if (!fn->params.has_static_chain) {
          stdout->write("Calling {}\n", s->name);
          stdout->flush();
          u64_t imms[]{l::lisp_i64(8).raw, l::lisp_i64(2).raw};
          l::lisp_imm res{s->value.as_fn_ref().value()(imms, sizeof(imms) / sizeof(u64_t))};
          stdout->write("Result (Full): {}\n", res);
        }
        } catch (...) {
          stdout->write("Oh no" );
        }

        // auto res = fn->env.call_abi(l::lisp_i64(1), l::lisp_i64(2));
        // stdout->write("Result (ABI):  {}\n", res);
        // stdout->flush();
      }

      auto repl_source = envc->get_repl_source();
      auto & src_compiler = input_pkg->get_eval_compiler(repl_source);
      auto bind = [&](l::lisp_imm imm) {
        stdout->write("  Form: {}\n", imm);
        src_compiler.compile_tlf(imm);
      };

      auto rd = l::reader::interactive_reader(&env, input_pkg, j::functions::bound_function<void (l::lisp_imm)>::bind<&decltype(bind)::operator()>(bind));
      for (auto & eval_string : eval) {
        stdout->write("Evaluating \"{#bold,gray_bg}{}{/}\"\n", eval_string);
        repl_source->src = eval_string;
        if (rd.read(repl_source) == l::reader::interactive_read_status::finished) {
          stdout->write("Finished\n");
        } else {
          stdout->write("Pending\n");
        }
      }
      auto sym = src_compiler.to_defun();
      if (sym->value_info.is_function()) {
        l::lisp_imm result = l::lisp_imm{sym->value.as_fn_ref().value()(nullptr, 0U)};
        stdout->write("Eval result", result);
      }
      rd.finalize();
      J_DUMP_TIMERS();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "commands.dump_cir",
      "Compile and Dump CIR",

      c::command_name = "dump-cir",
      c::callback     = s::call(&dump_cir,
                                s::service("stdout"),
                                c::flag("no-debug-print"),
                                c::flag("no-air-dumps"),
                                c::flag("no-cir-dumps"),
                                c::option("air-dump-path-pattern", c::default_value = "air.{}."),
                                c::option("cir-dump-path-pattern", c::default_value = "cir.{}.{}."),
                                c::option("include", attributes::is_multiple),
                                c::option("file", c::default_value = ""),
                                c::option("eval", attributes::is_multiple),
                                c::option("load", attributes::is_multiple),
                                c::argument("input", c::default_value = "")));
  }
}
