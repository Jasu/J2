#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/debug/graphviz_dump.hpp"
#include "logging/global.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/env/compilation_options.hpp"
#include "exceptions/assert.hpp"
#include "strings/format.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    using air_pass_instance = env::compilation_pass_instance<air_pass>;

    const air_pass_instance air_passes[]{
      { &basic_validation_pass, "validation-initial", "Validation: Initial" },

      { &trs_simplify_pass },
      { &count_exprs_pass, "count-exprs-trs-simplify", "Count expressions after TRS simplify" },
      { &basic_validation_pass, "validation-trs-simplify", "Validation after TRS simplify" },

      { &dce_pass, "dce-1", "DCE #1" },
      { &basic_validation_pass, "validation-dce-1", "Validation: Afer DCE #1" },

      { &reaching_defs_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-reaching-defs", "Validation: Afer reaching defs" },

      { &dce_pass, "dce-2", "DCE #2" },
      { &basic_validation_pass, "validation-dce-2", "Validation: Afer DCE #2" },

      { &lex_vars_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-lex-vars", "Validation after lex vars" },

      { &propagate_types_pass, "propagate-types-1", "Propagate types #1", env::pass_mandatory },
      { &basic_validation_pass, "validation-propagate-types-1", "Validation: After propagate types #1" },

      { &act_rec_compute_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-act-rec-compute", "Validation: Afer compute activation records" },

      { &act_rec_convert_pass, env::pass_mandatory },
      { &count_exprs_pass, "count-exprs-act-rec-convert", "Count expressions after convert activation records", env::pass_mandatory },
      { &basic_validation_pass, "validation-act-rec-convert", "Validation after convert activation records" },

      { &simplify_pass },
      { &count_exprs_pass, "count-exprs-simplify", "Count expressions after simplify", env::pass_mandatory },
      { &basic_validation_pass, "validation-simplify", "Validation: After simplify" },

      { &propagate_barriers_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-propagate-barriers", "Validation: After propagate barriers" },

      { &reassociate_pass },
      { &basic_validation_pass, "validation-reassociate", "Validation: After reassociate" },

      { &propagate_reps_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-propagate-reps", "Validation: After propagate reps" },

      { &convert_reps_pass, env::pass_mandatory },
      { &basic_validation_pass, "validation-convert-reps", "Validation: After convert reps" },

      { &propagate_types_pass, "propagate-types-2", "Propagate types #2", env::pass_mandatory },
      { &basic_validation_pass, "validation-propagate-types-2", "Validation: After propagate types #2" },
    };
  }

  air_pass_context::air_pass_context(compilation::context & ctx,
                                     strings::const_string_view dump_path,
                                     j::mem::bump_pool * J_NOT_NULL pool,
                                     air::air_function * J_NOT_NULL fn,
                                     const env::global_pass_options * J_NOT_NULL opts,
                                     debug_info_map * dbg) noexcept
    : env::compilation_pass_context_base(
          pool,
          dump_path,
          opts->dump_debug_files,
          opts->print_debug
      ),
      ctx(ctx),
      fn(fn),
      debug_info(dbg ? dbg : nullptr)
  { }

  void check_visit_flag(air::air_function * J_NOT_NULL fn, air::exprs::expr * J_NOT_NULL expr) {
    J_ASSERT(fn->visit_flag_value == expr->visit_flag);
    for (auto & in : expr->inputs()) {
      if (in.expr) {
        check_visit_flag(fn, in.expr);
      }
    }
  }

  void check_visit_flag(air::air_function * J_NOT_NULL fn) {
    check_visit_flag(fn, fn->body);
  }

  void run_air_passes(compilation::context & ctx, j::mem::bump_pool * J_NOT_NULL pool, air::air_function * J_NOT_NULL fn) {
    auto & opts = ctx.env_compiler.opts->air_options;
    if (opts.dump_initial_graph) {
      air::debug::graphviz_dump(
        strings::format(opts.pass_defaults.dump_file_pattern, "initial") + "tree.dot",
        fn);
    }

    debug_info_map * debug_info = nullptr;

    u32_t i = 0U;
    for (auto & pass : air_passes) {
      auto overrides = opts.per_pass.maybe_at(pass.get_name());
      if (!env::should_run_pass(pass, overrides)) {
        continue;
      }
      const env::global_pass_options * pass_opts = J_UNLIKELY(overrides) ? overrides : &opts.pass_defaults;

      // if (pass_opts->print_debug) {
      // J_DEBUG("{#cyan_bg,white} #{}: Entering AIR {#bold}{}{/}", i, pass->get_name());
      // }
        ++i;

      check_visit_flag(fn);

      if (pass->modifies_graph && pass_opts->dump_graph_after && !debug_info) {
        debug_info = J_ALLOCA_NEW(debug_info_map);
      }

      strings::string substituted_dump_path = strings::format(pass_opts->dump_file_pattern, pass.get_name());
      air_pass_context air_ctx(
        ctx, substituted_dump_path,
        pool, fn,
        pass_opts, pass_opts->dump_graph_after ? debug_info : nullptr);

      pass->execute(air_ctx);

      // if (pass_opts->print_debug) {
      //   J_DEBUG("{#cyan_bg,white} Leaving AIR {#bold}{}{/}", pass.get_name());
      // }

      if (pass->modifies_graph && pass_opts->dump_graph_after) {
        air::debug::graphviz_dump(air_ctx.get_dump_file_path("tree.dot"), fn, air_ctx.debug_info);
        debug_info->clear();
      }
    }
  }
}
