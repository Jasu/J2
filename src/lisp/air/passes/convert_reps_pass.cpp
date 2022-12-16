#include "lisp/air/passes/air_passes.hpp"
#include "logging/global.hpp"
#include "exceptions/assert.hpp"
#include "lisp/air/exprs/accessors.hpp"
#include "lisp/air/exprs/rep_spec_mode.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "lisp/air/values/rep_counts.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN convert_reps_visitor final {
      exprs::expr_builder builder;

      enum rep maybe_add_convert(exprs::expr & e, rep_spec requirement, enum rep rep) const noexcept {
        J_ASSERT(requirement && (rep != rep_none));
        if (requirement.has(rep)) {
          e.result.reps.set_rep(rep);
          return rep;
        }

        enum rep to = requirement.get_best();
        J_ASSERT(to != rep_none);
        e.result.reps.set_rep(e.result.reps.get_best());

        exprs::expr * convert;
        if (e.result.types == imm_range && to != rep_range) {
          convert = builder.wrap_with(&e, expr_type::as_vec);
          rep = to;
        } else if (rep == rep_range) {
          convert = builder.wrap_with(&e, expr_type::as_vec);
          rep = to;
        } else if (to == rep_range) {
          convert = builder.wrap_with(&e, expr_type::as_range);
          rep = to;
        } else if (to == rep_untagged) {
          convert = builder.wrap_with(&e, expr_type::untag);
          rep = to;
        } else if (to == rep_tagged) {
          convert = builder.wrap_with(&e, expr_type::tag);
          rep = to;
        } else {
          J_FAIL("Unsupported rep");
        }
        convert->visit_flag = builder.fn.visit_flag_value;
        if (convert->type != expr_type::as_vec) {
          convert->result.types = e.result.types;
        }
        convert->input(0).type.types = e.result.types;
        convert->result.reps.set_rep(rep);
        return rep;
      }

      rep visit_in(exprs::expr & e, u8_t index, rep_spec requirement) {
        J_ASSERT(index < e.num_inputs);
        rep result = rep_none;
        auto & in = e.input(index);
        result = visit(*in.expr, in.type.reps & requirement);
        result = maybe_add_convert(e, requirement, result);

        for (u8_t i = 0; i < e.num_inputs; ++i) {
          if (index != i) {
            auto & in = e.input(i);
            visit(*in.expr, in.type.reps);
          }
        }
        return result;
      }

      rep visit_inputs_same(exprs::expr & e, rep_spec requirement, bool all_same) {
        rep_counts counts;
        if (all_same) {
          counts.add(requirement);
        }
        for (auto & in : e.inputs()) {
          counts.add(in.expr->result.reps & in.type.reps);
        }
        rep rep = counts.get_max();
        requirement.set_mask(rep);
        for (auto & in : e.inputs()) {
          visit(*in.expr, requirement & in.type.reps);
        }
        return all_same ? rep : e.result.rep();
      }

      rep visit(exprs::expr & e, rep_spec requirement) {
        J_ASSERT(requirement);
        if (e.visit_flag == builder.fn.visit_flag_value) {
          return e.result.rep();
        }
        e.visit_flag = builder.fn.visit_flag_value;

        rep result = rep_none;

        switch (exprs::rep_spec_modes[(u8_t)e.type]) {
        case exprs::rep_spec_mode::lexvar: {
          exprs::lex_rd_accessor a{e};
          if (e.result.reps.has_rep()) {
            result = e.result.rep();
          } else {
            result = e.result.reps.get_best();
          }
          rep_spec new_req(result);
          result = maybe_add_convert(e, requirement, result);
          for (auto def : a.reaching_writes()) {
            // Loop - don't cause an error when trying to enter a pending node.
            if (def->visit_flag == builder.fn.visit_flag_value) {
              continue;
            }
            visit(*def, new_req);
          }
          return result;
        }
        case exprs::rep_spec_mode::inputs_same:
          result = (requirement & e.result.reps).get_best();
          if (result == rep_none) {
            result = e.result.reps.get_best();
            J_ASSERT(result != rep_none);
          }
          result = maybe_add_convert(e, requirement, result);
          visit_inputs_same(e, rep_spec::any, false);
          return result;
        case exprs::rep_spec_mode::all_same_rep_and_type:
        case exprs::rep_spec_mode::all_same_rep:
          result = visit_inputs_same(e, requirement, true);
          break;
        case exprs::rep_spec_mode::branch_result: {
          u8_t i = 0U;
          rep_counts counts;
          counts.add(e.result.reps);
          i = 0U;
          for (auto & in : e.inputs()) {
            if (is_branch(e.type, i) && in.expr) {
              counts.add(requirement);
            }
            ++i;
          }
          result = counts.get_max();
          result = maybe_add_convert(e, requirement, result);
          i = 0U;
          for (auto & in : e.inputs()) {
            if (!is_branch(e.type, i)) {
              visit(*in.expr, in.type.reps);
            }
            ++i;
          }
          rep_spec new_req(result);
          i = 0U;
          for (auto & in : e.inputs()) {
            if (is_branch(e.type, i) && in.expr) {
              visit(*in.expr, new_req);
            }
            ++i;
          }
        }
          return result;
        case exprs::rep_spec_mode::type_in0:
        case exprs::rep_spec_mode::type_in1:
        case exprs::rep_spec_mode::type_in2:
        case exprs::rep_spec_mode::constant:
          result = (requirement & e.result.reps).get_best();
          if (result == rep_none) {
            result = e.result.reps.get_best();
            J_ASSERT(result != rep_none);
          }
          result = maybe_add_convert(e, requirement, result);
          for (auto & in : e.inputs()) {
            visit(*in.expr, in.type.reps);
          }
          return result;
        case exprs::rep_spec_mode::progn:
          result = visit_in(e, e.num_inputs - 1, requirement);
          break;
        case exprs::rep_spec_mode::in0:
          result = visit_in(e, 0, requirement);
          break;
        case exprs::rep_spec_mode::in1:
          result = visit_in(e, 1,requirement);
          break;
        case exprs::rep_spec_mode::in2:
          result = visit_in(e, 2, requirement);
          break;
        }
        return maybe_add_convert(e, requirement, result);
      }
    };

    void apply_convert_reps(air_pass_context & ctx) {
      ctx.fn->visit_flag_value = !ctx.fn->visit_flag_value;
      convert_reps_visitor v{
        .builder{ctx.pool, *ctx.fn},
      };
      v.visit(*ctx.fn->body, rep_spec::tagged);
    }
  }

  const air_pass convert_reps_pass{apply_convert_reps, "convert-reps", "Convert reps", true};
}
