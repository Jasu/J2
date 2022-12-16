#include "lisp/air/passes/air_passes.hpp"
#include "logging/global.hpp"
#include "lisp/air/exprs/rep_spec_mode.hpp"
#include "exceptions/assert.hpp"
#include "lisp/air/exprs/accessors.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN propagate_types_visitor final {
      exprs::expr_builder builder;

      void maybe_add_type_check(exprs::expr & e, imm_type_mask requirement) const noexcept {
        requirement &= e.result.types;
        if (e.result.types != requirement && e.result.types && requirement) {
          auto check = builder.wrap_with(&e, expr_type::dyn_type_assert, imm_type_mask(requirement.value));
          check->visit_flag = builder.fn.visit_flag_value;
        }
      }

      imm_type_mask propagate_indexed(exprs::expr & e, imm_type_mask requirement, u8_t index) const noexcept {
        J_ASSERT(index < e.num_inputs);
        for (u8_t i = 0; i < e.num_inputs; ++i) {
          auto & in = e.input(i);
          if (index == i) {
            e.result.types = propagate(*in.expr, in.type.types & requirement);
            if (!e.result.reps.has_rep()) {
              e.result.reps &= get_possible_reps(e.result.types);
              J_ASSERT(e.result.reps, "Reps failed", e.type, e.result.types);
            }
          } else {
            propagate(*in.expr, in.type.types);
          }
        }
        return e.result.types;
      }

      imm_type_mask visit_inputs(exprs::expr & e) const noexcept {
        imm_type_mask result;
        for (auto & in : e.inputs()) {
          result |= propagate(*in.expr, in.type.types);
        }
        return result;
      }

      imm_type_mask propagate_inputs(exprs::expr & e, imm_type_mask requirement) const noexcept {
        imm_type_mask result;
        for (auto & in : e.inputs()) {
          result |= propagate(*in.expr, in.type.types & requirement);
        }
        return result;
      }

      imm_type_mask propagate(exprs::expr & e, imm_type_mask requirement) const noexcept {
        if (builder.fn.visit_flag_value == e.visit_flag) {
          return e.result.types;
        }
        e.visit_flag = builder.fn.visit_flag_value;
        imm_type_mask result;

        switch (exprs::rep_spec_modes[(u8_t)e.type]) {
        case exprs::rep_spec_mode::lexvar: {
          exprs::lex_rd_accessor a{e};
          for (auto def : a.reaching_writes()) {
            J_ASSUME_NOT_NULL(def);
            result |= propagate(*def, any_imm_type);
          }
          if (!result) {
            result = e.result.types;
          }
        }
          break;
        case exprs::rep_spec_mode::constant:
        case exprs::rep_spec_mode::inputs_same:
        case exprs::rep_spec_mode::all_same_rep:
          visit_inputs(e);
          result = e.result.types;
          break;
        case exprs::rep_spec_mode::all_same_rep_and_type:
          result = propagate_inputs(e, requirement);
          break;
        case exprs::rep_spec_mode::branch_result: {
          u8_t i = 0;
          for (auto b : e.inputs()) {
            if (b) {
              auto subresult = propagate(*b.expr, requirement);
              if (is_branch(e.type, i)) {
                result |= subresult;
              }
            }
            ++i;
          }
        }
          break;
        case exprs::rep_spec_mode::progn:
          result = propagate_indexed(e, requirement, e.num_inputs - 1);
          break;
        case exprs::rep_spec_mode::in0:
        case exprs::rep_spec_mode::type_in0:
          result = propagate_indexed(e, requirement, 0);
          break;
        case exprs::rep_spec_mode::in1:
        case exprs::rep_spec_mode::type_in1:
          result = propagate_indexed(e, requirement, 1);
          break;
        case exprs::rep_spec_mode::in2:
        case exprs::rep_spec_mode::type_in2:
          result = propagate_indexed(e, requirement, 2);
          break;
        }
        e.result.types = result;
        if (!e.result.reps.has_rep()) {
          e.result.reps &= get_possible_reps(result);
          J_ASSERT(e.result.reps, "Reps failed", e.type, result);
        }
        requirement &= result;
        maybe_add_type_check(e, requirement);
        return requirement;
      }
    };

    void apply_propagate_types(air_pass_context & ctx) {
      ctx.fn->visit_flag_value = !ctx.fn->visit_flag_value;
      propagate_types_visitor v{
        .builder{ctx.pool, *ctx.fn},
      };
      v.propagate(*ctx.fn->body, taggable_imm_type);
    }
  }

  const air_pass propagate_types_pass{apply_propagate_types, "propagate-types", "Propagate types", true};
}
