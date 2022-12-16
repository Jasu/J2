#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/cir/debug/graphviz_dump.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"

namespace j::lisp::cir::inline passes {
  namespace {
    using cir_pass_instance = env::compilation_pass_instance<cir_pass>;
    const cir_pass_instance cir_passes[]{
      { &reverse_postorder_pass, "reverse-postorder-1", "BB reverse post-order #1", env::pass_mandatory },
      { &full_call_pass, env::pass_mandatory },
      { &legalize_pass, env::pass_mandatory },
      { &number_ops_pass, env::pass_mandatory },
      { &def_use_pass, env::pass_mandatory },
      { &cir::liveness::compute_live_ranges_pass, env::pass_mandatory },
      { &cir::reg_alloc::allocate_regs_pass, env::pass_mandatory },
      { &cir::reg_alloc::resolve_phis_pass, env::pass_mandatory },
      { &reverse_postorder_pass, "reverse-postorder-2", "BB reverse post-order #2", env::pass_mandatory },
      { &remove_empty_bbs_pass }
    };
  }

  cir_pass_context::cir_pass_context(compilation::context & ctx,
                                  strings::const_string_view dump_path,
                                  j::mem::bump_pool * J_NOT_NULL pool,
                                  functions::func_info * func_info,
                                  struct ssa_builder * J_NOT_NULL builder,
                                  const env::global_pass_options * J_NOT_NULL opts,
                                  functions::calling_convention cc) noexcept
    : env::compilation_pass_context_base(pool, dump_path, opts->dump_debug_files, opts->print_debug),
      compilation_context(ctx),
      func_info(func_info),
      target(ctx.env_compiler.opts->target),
      ssa(builder->ssa),
      ssa_builder(builder),
      calling_convention(cc)
  { }

  void run_cir_passes(compilation::context & ctx,
                      j::mem::bump_pool * J_NOT_NULL pool,
                      functions::func_info * func_info,
                      functions::calling_convention cc,
                      cir::ssa_builder * J_NOT_NULL ssa_builder) {
    J_ASSUME(cc != functions::calling_convention::none);
    auto & opts = ctx.env_compiler.opts->cir_options;
    const char * const cc_name = (cc == functions::calling_convention::full_call) ? "full_call" : "abi_call";

    if (opts.dump_initial_graph) {
      cir::debug::graphviz_dump(
        strings::format(opts.pass_defaults.dump_file_pattern, cc_name, "initial") + "ssa.dot",
        *ssa_builder->ssa);
    }

    for (auto & pass : cir_passes) {
      const env::resolved_pass_options * overrides = opts.per_pass.maybe_at(pass.get_name());
      if (!env::should_run_pass(pass, overrides)) {
        continue;
      }


      const env::global_pass_options * pass_opts = J_UNLIKELY(overrides) ? overrides : &opts.pass_defaults;

      if (!pass->enabled_calling_conventions.has(cc)) {
        if (pass_opts->print_debug) {
          J_DEBUG("Skipping CIR pass {} due to CC.", pass.get_name());
        }
        continue;
      }
      strings::string substituted_dump_path = strings::format(pass_opts->dump_file_pattern, cc_name, pass.get_name());
      const cir_pass_context cir_ctx{ctx, substituted_dump_path, pool, func_info, ssa_builder, pass_opts, cc};

      pass->execute(cir_ctx);

      if (pass_opts->dump_graph_after) {
        cir::debug::graphviz_dump(cir_ctx.get_dump_file_path("ssa.dot"), *ssa_builder->ssa);
      }
    }
  }
}
