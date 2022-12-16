#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/exprs/substitutions.hpp"
#include "lisp/env/context.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "lisp/air/exprs/expr_data.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/air/passes/const_fold.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    using f = exprs::expr_flag;

    struct J_TYPE_HIDDEN simplify_visitor final {
      compilation::context & ctx;
      exprs::expr_builder builder;
      const const_fold_table & folds;


      J_RETURNS_NONNULL exprs::expr * replace_with_const(exprs::expr * e, lisp_imm val) {
        return builder.create_late(e->parent, e->parent_pos, expr_type::ld_const, val);
      }

      void visit(exprs::expr * J_NOT_NULL e) {
      do_again:
        auto & data = exprs::expr_data_v[e->type];
        auto & flags = data.flags;
        auto first_input = e->num_inputs ? e->input(0).expr : nullptr;
        const bool is_prognish = flags.has(exprs::expr_flag::prognish);

        // Move progns up. (+ (progn (foo) 0) 2) -> (progn (foo) (+ 0 2)) which allows optimizing the + expression.
        if (first_input) {
          if (is_prognish) {
            for (u32_t i = 0U; i < e->num_inputs - 1U;) {
              auto in = e->input(i).expr;
              if (exprs::expr_data_v[in->type].flags & exprs::expr_flags_non_elidable_v) {
                ++i;
                continue;
              }
              auto new_e = builder.try_merge_inputs_to_parent(in);
              if (!new_e && e->type == expr_type::fn_body) {
                ++i;
                continue;
              }
              e = new_e;
              J_ASSERT(e, "Merging prognish failed.");

              if (!i) {
                if (!e->num_inputs) {
                  first_input = nullptr;
                  break;
                }
                first_input = e->input(0).expr;
              }
            }
          } else if (e->type != expr_type::do_until) {
            // Replace 1-element progns, to avoid redundant work.
            if (first_input->type == expr_type::progn && first_input->num_inputs == 1) {
              builder.attach_to(e, 0, first_input->input(0).expr);
              goto do_again;
            } else if (first_input->num_inputs && (first_input->type == expr_type::progn || first_input->type == expr_type::let)) {
              auto progn_last_idx = first_input->num_inputs - 1;
              auto progn_last = first_input->input(progn_last_idx).expr;
              builder.attach_to(e->parent, e->parent_pos, first_input);
              builder.attach_to(first_input, progn_last_idx, e);
              builder.attach_to(e, 0, progn_last);
              e = first_input;
              goto do_again;
            }
          }
        }

        if (first_input) {
          // Two identical inputs nested.
          // Handle self-cancelling and composition-idempotent cases.
          if (e->num_inputs == 1U && first_input->type == e->type) {
            // Handle self-cancelling
            if (flags.has(f::involutory) && first_input->num_inputs == 1U) {
              auto expr = first_input->input(0).expr;
              builder.attach_to(e->parent, e->parent_pos, expr);
              e = expr;
              goto do_again;
            } else if (flags.has(f::composition_idempotent)) {
              // Handle
              //  - (abs (abs x)) -> x
              //  - (copy (copy x))
              //  - (concat (concat x))
              builder.attach_to(e->parent, e->parent_pos, first_input);
              e = first_input;
              goto do_again;
            }
          }

          for (u32_t i = 0U; i < e->num_inputs; ++i) {
            visit(e->input(i).expr);
          }

          // Handle
          // (+ 1 2 0 3) -> (+ 1 2 3)
          // (- 1 2 0 3) -> (- 1 2 3) but not (- 0 1 2)
          bool is_unary_identity = flags.has(f::unary_identity);
          bool has_identity = false;//flags.has(f::has_identity_element);
          bool has_identity_tail = flags.has(f::has_identity_element_tail);
          if ((e->num_inputs && has_identity) || (e->num_inputs > 1 && has_identity_tail)) {
            for (u32_t i = has_identity_tail ? 1U : 0U; i < e->num_inputs;) {

              // Don't collapse (/ 1) to (/) which would be invalid.
              // Same applies to (- 0) to (-).
              if (is_unary_identity && e->num_inputs == 1) {
                break;
              }
              auto in = e->input(i).expr;
              if (in->type == expr_type::ld_const && in->get_const().eq(data.identity_element)) {
                builder.erase_operand(*e, i);
              } else {
                ++i;
              }
            }
          }


          // Handle
          // (* x) -> x, (/ x) -> x
          // (+ x) -> x, (- x) -> x
          // Note that in the last case, (- x) is (isub x) which is not enterable
          // in reader - it occurs only when some optimization deletes operands.
          // (progn x) -> x
          if (e->num_inputs == 1U && is_unary_identity) {
            builder.attach_to(e->parent, e->parent_pos, e->input(0).expr);
            return;
          }

          auto & fold = folds[e->type];
          if (e->num_inputs == 1 && fold.is_foldable_unary()) {
            auto in = e->input(0).expr;
            if (in->type == expr_type::ld_const) {
              replace_with_const(e, fold.fold(in->get_const()));
              return;
            }
          } else if (e->num_inputs == 2 && fold.is_foldable_binary()) {
            auto lhs = e->input(0).expr;
            auto rhs = e->input(1).expr;
            if (lhs->type == expr_type::ld_const && rhs->type == expr_type::ld_const) {
              replace_with_const(e, fold.fold(lhs->get_const(), rhs->get_const()));
              return;
            }
          } else if (e->num_inputs > 2 && fold.is_foldable_binary()) {
            auto lhs = e->input(0).expr, rhs = e->input(1).expr;

            while (lhs->type == expr_type::ld_const && rhs->type == expr_type::ld_const) {
              if (e->num_inputs == 2) {
                replace_with_const(e, fold.fold(lhs->get_const(), rhs->get_const()));
                return;
              }
              lhs = replace_with_const(lhs, fold.fold(lhs->get_const(), rhs->get_const()));
              builder.erase(*rhs);
              rhs = e->input(1).expr;
            }

            for (u8_t i = 1; i < e->num_inputs;) {
              auto rhs = e->input(i).expr;
              if (lhs->type == expr_type::ld_const && rhs->type == expr_type::ld_const) {
                lhs = replace_with_const(lhs, fold.fold_tail(lhs->get_const(), rhs->get_const()));
                builder.erase(*rhs);
                continue;
              }
              lhs = rhs;
              ++i;
            }
            if (e->num_inputs == 1) {
              goto do_again;
            }
          }
        }

        // Handle
        // (eq) -> true, (neq) -> false
        // (+) -> 0, (-) -> 0
        // (*) -> 1
        // (progn) -> nil
        // (or) -> nil, (and) -> true
        if (!e->num_inputs && flags.has(f::is_nullary_constant)) {
          replace_with_const(e, data.nullary_result);
          return;
        }

        // (eq x) -> true
        // (neq x) -> false
        if (e->num_inputs == 1U && flags.has(f::is_unary_constant)) {
          e = builder.create_late(e->parent, e->parent_pos, expr_type::progn,
                              span<exprs::expr* const>({
                                  e->input(0).expr,
                                  builder.create(expr_type::ld_const, data.unary_result)
                                }));
          goto do_again;
        }
        span<const exprs::substitution> substs;
        switch (e->type) {
        default: break;
        case expr_type::let: substs = exprs::substitutions::let_group; break;
        // case expr_type::range_length: substs = exprs::substitutions::range_length_group; break;
        // case expr_type::sym_val_rd: substs = exprs::substitutions::sym_val_rd_group; break;
        // case expr_type::lor: substs = exprs::substitutions::lor_group; break;
        // case expr_type::land: substs = exprs::substitutions::land_group; break;
        // case expr_type::isub: substs = exprs::substitutions::isub_group; break;
        }

        for (auto & tx : substs) {
          if (auto resolved = tx.set.try_resolve(e)) {
            e = resolved.run(builder);
            goto do_again;
          }
        }
      }
    };

    void apply_simplify_pass(air_pass_context & ctx) {
      simplify_visitor v{
        .ctx{ctx.ctx},
        .builder{ctx.pool, *ctx.fn},
        .folds = *get_const_fold_table(env::env_context->env),
      };
      v.visit(ctx.fn->body);
    }
  }

  const air_pass simplify_pass{apply_simplify_pass, "simplify", "Simplify expressions", true};
}
