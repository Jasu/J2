#include "utils.hpp"
#include "lisp/assembly/amd64/abi/ia64_target.hpp"
#include "logging/global.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/packages/pkg.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "lisp/assembly/disassembly/disassembler.hpp"

namespace j::lisp::tests {
  namespace l = j::lisp;
  namespace a = j::lisp::assembly;
  namespace str = j::strings;
    void dump_fn(const a::target * J_NOT_NULL tgt,
                 str::const_string_view name,
                 const l::functions::func_info * J_NOT_NULL fn_info,
                 l::functions::calling_convention cc)
    {
      j::logging::global_logger::instance.ensure_sink();
      auto stdout = j::logging::global_logger::instance.m_sink;
      // TODO allocation not set for closures, fix that so they are dumped.
      if (!fn_info->cc_info[cc] || !fn_info->cc_info[cc].allocation) {
        return;
      }
      auto dis = tgt->disassembler();
      stdout->write("{} @ 0x{:016X}:\n\n", name, fn_info->cc_info[cc].fn_ptr);
      dis->disassemble_buffer(
        fn_info->cc_info[cc].allocation.code_region(),
        fn_info->cc_info[cc].asm_info,
        [&stdout](const l::assembly::disassembly::instruction & inst) {
          stdout->write("  {}\n", inst);
        });
    }

  j::mem::shared_ptr<env::environment> make_env() {
    return ::j::mem::make_shared<env::environment>();
  }

  inline lisp_imm run_code(const j::mem::shared_ptr<env::environment> & e, const char * J_NOT_NULL code, const char * J_NOT_NULL fn, imms_t args) {
    auto guard = env::env_context.enter(e.get());
    auto tgt = assembly::target::get_target(assembly::target_name::amd64_ia64);
    auto asmctx = assembly::asm_context.enter(tgt);
    env::compilation_options opts = env::compilation_options_nodebug;
    // opts.air_options.pass_defaults.dump_graph_after = true;
    // opts.air_options.pass_defaults.dump_debug_files = true;
    // opts.air_options.dump_initial_graph = true;
    // opts.cir_options.pass_defaults.dump_graph_after = true;
    // opts.cir_options.pass_defaults.dump_debug_files = true;
    // opts.cir_options.dump_initial_graph = true;
    // opts.air_options.pass_defaults.dump_file_pattern = "sphinx/source/test-dump/air.{}.";
    // opts.cir_options.pass_defaults.dump_file_pattern = "sphinx/source/test-dump/cir.{}.{}.";
    opts.target = tgt;
    env::env_compiler * envc = e->get_env_compiler(&opts);
    auto input_pkg = envc->compile_pkg("input", envc->source_manager.add_static_source("input", code));
    input_pkg->load();
    l::id id = input_pkg->id_of(fn);
    REQUIRE_UNARY(id);
    auto fnp = input_pkg->get_fn(id);
    REQUIRE_UNARY((bool)fnp);
    INFO((void*)fnp.fn);
    CHECK(j::is_aligned((void*)fnp.fn, 16));
    auto s = input_pkg->symbol_table.at(id);
    REQUIRE_UNARY(s);
    REQUIRE_UNARY(s->is_initialized());
    // dump_fn(tgt, fn, s->value_info.func_info, l::functions::calling_convention::abi_call);
    return fnp(args);
  }

  lisp_imm run_code(const j::mem::shared_ptr<env::environment> & e, const char * J_NOT_NULL code, const char * J_NOT_NULL fn) {
    return run_code(e, code, fn, imms_t{});
  }
}
