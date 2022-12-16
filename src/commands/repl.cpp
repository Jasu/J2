#include "cli/cli.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"
#include "lisp/assembly/amd64/abi/ia64_target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/assembly/disassembly/disassembler.hpp"
#include "lisp/compilation/source_compilation.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/lisp_fwd.hpp"
#include "lisp/sources/source.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/reader/interactive_reader.hpp"
#include "services/service_instance.hpp"

#include "events/event_loop.hpp"
#include "strings/styling/default_styles.hpp"
#include "tty/components/divider.hpp"
#include "tty/components/scroll_view.hpp"
#include "tty/components/static_string.hpp"
#include "tty/components/line_editor.hpp"
#include "tty/components/linear_layout.hpp"
#include "tty/uv_termpaint_integration.hpp"

namespace j::commands {
  namespace {
    namespace l = j::lisp;
    namespace str = j::strings;

    void repl(mem::shared_ptr<j::events::event_loop> event_loop,
              bool no_debug_print,
              str::const_string_view air_dump_path_pattern,
              str::const_string_view cir_dump_path_pattern,
              trivial_array<files::path> include_paths) {
      logging::global_logger::instance.set_sink_to_file("debug.log");

      l::env::environment env;
      auto guard = l::env::env_context.enter(&env);
      auto tgt = l::assembly::target::get_target(l::assembly::target_name::amd64_ia64);
      auto asmctx = l::assembly::asm_context.enter(tgt);
      l::env::compilation_options opts(tgt,
                                       include_paths
                                       ? static_cast<trivial_array<files::path> &&>(include_paths)
                                       : trivial_array<files::path>(
                                         containers::move, { files::path(".") }),
                                       true);

      if (no_debug_print) {
        opts.air_options.pass_defaults.print_debug = false;
        opts.cir_options.pass_defaults.print_debug = false;
      }
      if (air_dump_path_pattern) {
        opts.air_options.pass_defaults.dump_file_pattern = air_dump_path_pattern;
      }
      if (cir_dump_path_pattern) {
        opts.cir_options.pass_defaults.dump_file_pattern = cir_dump_path_pattern;
      }
      l::env::env_compiler * envc = env.get_env_compiler(&opts);

      // auto dis = tgt->disassembler();

      l::packages::pkg * input_pkg = nullptr;

      input_pkg = envc->create_pkg("input");
      input_pkg->load();
      auto repl_source = envc->get_repl_source();
      auto & src_compiler = input_pkg->get_eval_compiler(repl_source);

      auto bind = [&](l::lisp_imm imm) {
        J_DEBUG("Compliing", imm);
        src_compiler.compile_tlf(imm);
      };
      auto rd = l::reader::interactive_reader(&env, input_pkg, j::functions::bound_function<void (l::lisp_imm)>::bind<&decltype(bind)::operator()>(bind));

      tty::uv_termpaint_integration uvi(event_loop, 0);
      uvi.initialize();

      tty::static_string result("No result");

      tty::divider divider;
      tty::static_string pend("");
      pend.set_text_style(strings::styling::styles::light_gray);
      pend.set_prefix_style(strings::styling::styles::bright_yellow);
      pend.table.prefix = ": ";
      pend.table.initial_prefix = "  ";
      divider.set_style(strings::styling::styles::bright_magenta);

      tty::line_editor le(&uvi);
      le.set_prompt_style(strings::styling::styles::bold);
      le.set_continuation_style(strings::styling::styles::light_gray);

      auto accept = [&](strings::string && s) {
        strings::string source = s + "\n";
        repl_source->src = source;
        auto read_ready = rd.read(repl_source) == l::reader::interactive_read_status::finished;
        auto compile_ready = read_ready && src_compiler.num_pending_tlfs == 0;
        if (compile_ready) {
          auto sym = src_compiler.to_defun();
          if (sym->value_info.is_function()) {
            l::lisp_imm eval_res = l::lisp_imm{sym->value.as_fn_ref().value()(nullptr, 0U)};
            result.set_text(strings::format("{}", eval_res));
            input_pkg->clear_eval_symbol();
          }
          pend.set_text("");
        } else {
          if (pend.table.value.empty()) {
            pend.set_text("Pending");
          }
          pend.set_text((strings::string)(strings::rope_utf8_view)pend.table.value + "\n" + source.without_suffix(1));
          if (read_ready) {
            result.set_text("Pending for symbol definitions");
          } else {
            result.set_text("Pending for syntax");
          }
        }
        le.set_text("");
      };
      le.on_accept.connect<&decltype(accept)::operator()>(&accept);

      tty::linear_layout lin(tty::axis::vertical);
      lin.append_child(&result);
      lin.append_child(&divider);
      lin.append_child(&pend);
      lin.append_child(&le);
      tty::scroll_view sv(tty::axis::vertical, tty::scroll_mode::bottom, &lin);

      uvi.set_root(&sv);
      event_loop->run();

      // rd.read(repl_source);
      // rd.finalize();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "commands.repl",
      "Compile and Dump CIR",

      c::command_name = "repl",
      c::callback     = s::call(&repl,
                                s::arg::autowire,
                                c::flag("no-debug-print"),
                                c::option("air-dump-path-pattern", c::default_value = "air.{}."),
                                c::option("cir-dump-path-pattern", c::default_value = "cir.{}.{}."),
                                c::option("include", attributes::is_multiple)));
  }
}
