#include "lisp/air/passes/air_passes.hpp"
#include "logging/global.hpp"

#include "lisp/air/exprs/accessors.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/exprs/rep_spec_mode.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/values/rep_counts.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_A(HIDDEN) propagate_reps_visitor final {
      bool visit_flag;

      rep_spec propagate_in(exprs::expr & e, u8_t index, rep_spec requirement) {
        J_ASSERT(index < e.num_inputs);
        rep_spec result;
        for (u8_t i = 0; i < e.num_inputs; ++i) {
          auto & in = e.input(i);
          if (index == i) {
            result = propagate(*in.expr, in.type.reps & requirement);
          } else {
            propagate(*in.expr, in.type.reps);
          }
        }
        return result;
      }

      rep_spec propagate_same(exprs::expr & e, rep_spec requirement, bool all_same) noexcept {
        rep_spec child_requirement = all_same ? requirement : rep_spec::any;
        rep_counts counts;
        for (auto & in : e.inputs()) {
          child_requirement &= in.type.reps;
          counts.add(in.type.reps & in.expr->result.reps);
        }
        if (all_same) {
          counts.add(requirement);
        }
        child_requirement.set_hint(counts.get_max());
        counts.reset();
        for (auto & in : e.inputs()) {
          counts.add(propagate(*in.expr, in.type.reps & child_requirement));
        }
        if (all_same) {
          child_requirement.set_hint(counts.get_max());
          return child_requirement;
        }
        return e.result.reps;
      }

      rep_spec propagate(exprs::expr & e, rep_spec requirement) {
        J_ASSERT(requirement);
        if (e.visit_flag == visit_flag) {
          return e.result.reps;
        }
        e.visit_flag = visit_flag;

        rep_spec result;
        auto mode = exprs::rep_spec_modes[(u8_t)e.type];
        switch (mode) {
        case exprs::rep_spec_mode::lexvar: {
          exprs::lex_rd_accessor a{e};
          if (e.result.reps.has_rep()) {
            // J_DEBUG("Has rep already", e);
            result = e.result.reps;
          } else {
            rep_counts counts;
            counts.add(requirement);
            for (auto def : a.reaching_writes()) {
              counts.add(def->result.reps);
            }
            requirement.set_hint(counts.get_max());

            counts.reset();
            for (auto def : a.reaching_writes()) {
              rep_spec subresult = propagate(*def, requirement);
              counts.add(subresult);
              result |= subresult;
            }
            result.set_hint(counts.get_max());
          }
        }
          break;
        case exprs::rep_spec_mode::inputs_same:
        case exprs::rep_spec_mode::all_same_rep_and_type:
        case exprs::rep_spec_mode::all_same_rep:
          result = propagate_same(e, requirement, mode == exprs::rep_spec_mode::inputs_same);
          break;
        case exprs::rep_spec_mode::branch_result:
          for (u8_t i = 0U; i < e.num_inputs; ++i) {
            auto & in = e.input(i);
            if (is_branch(e.type, i)) {
              result |= propagate(*in.expr, requirement & in.type.reps);
            } else {
              propagate(*in.expr, in.type.reps);
            }
          }
          break;
        case exprs::rep_spec_mode::type_in0:
        case exprs::rep_spec_mode::type_in1:
        case exprs::rep_spec_mode::type_in2:
        case exprs::rep_spec_mode::constant: {
          for (auto & in : e.inputs()) {
            propagate(*in.expr, in.type.reps);
          }
          result = e.result.reps & requirement;
        }
          break;
        case exprs::rep_spec_mode::progn:
          result = propagate_in(e, e.num_inputs - 1, requirement);
          break;
        case exprs::rep_spec_mode::in0:
          result = propagate_in(e, 0, requirement);
          break;
        case exprs::rep_spec_mode::in1:
          result = propagate_in(e, 1,requirement);
          break;
        case exprs::rep_spec_mode::in2:
          result = propagate_in(e, 2, requirement);
          break;
        }
        if (result & e.result.reps) {
          e.result.reps = result & e.result.reps;
        }
        return result;
      }
    };

    void apply_propagate(air_pass_context & ctx) {
      ctx.fn->visit_flag_value = !ctx.fn->visit_flag_value;
      propagate_reps_visitor v{ctx.fn->visit_flag_value};
      v.propagate(*ctx.fn->body, rep_spec::tagged);
    }
  }

  const air_pass propagate_reps_pass{apply_propagate, "propagate-reps", "Propagate reps"};
}
