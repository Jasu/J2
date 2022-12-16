#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/exprs/visit.hpp"
#include "lisp/air/exprs/expr_builder.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    enum class progn_mode : u8_t {
      none,
      progn,
      prog1,
    };
    enum class action : u8_t {
      keep,
      remove,
      convert_to_progn,
    };

    using expr = air::exprs::expr;
    namespace e = air::exprs;

    struct J_TYPE_HIDDEN dce_visitor {
      air::exprs::expr_builder ctx;
      const u32_t * var_uses_rd = nullptr;
      u32_t * var_uses_wr = nullptr;
      bool did_remove_var = false;

      u8_t count_inputs(const expr * J_NOT_NULL e) const noexcept {
        u8_t result = 0U;
        for (auto & i : e->inputs()) {
          if (i.expr && i.expr->type != expr_type::nop) {
            ++result;
          }
        }
        return result;
      }

      J_INLINE_GETTER action operator()(e::nop *, bool) const noexcept {
        return action::keep;
      }

      action operator()(e::fn_body * e, bool) noexcept {
        handle_operands(e, true, progn_mode::progn);
        return action::keep;
      }

      action operator()(e::do_until * e, bool) noexcept {
        handle_operands(e, true, progn_mode::progn);
        return action::keep;
      }

      template<OneOf<
        e::fn_arg, e::fn_sc_arg, e::fn_rest_arg,
        e::ld_const, e::sym_val_rd,
        e::act_rec_alloc, e::act_rec_parent, e::act_rec_rd> E>
      J_INLINE_GETTER_NO_DEBUG action operator()(E * J_NOT_NULL, bool result_used) const noexcept {
        return result_used ? action::keep : action::remove;
      }

      template<OneOf<e::lex_rd, e::lex_rd_bound> E>
      action operator()(E * J_NOT_NULL e, bool result_used) const noexcept {
        if (result_used) {
          ++var_uses_wr[e->var->index];
        }
        return result_used ? action::keep : action::remove;
      }

      action operator()(e::lex_wr_bound * J_NOT_NULL e, bool) noexcept {
        ++var_uses_wr[e->var->index];
        handle_operands(e, true);
        return action::keep;
      }

      action operator()(e::lex_wr * J_NOT_NULL e, bool result_used) noexcept {
        if (var_uses_rd && !var_uses_rd[e->var->index]) {
          handle_operands(e, result_used, progn_mode::progn);
          did_remove_var = true;
          return result_used ? action::convert_to_progn : action::remove;
        } else {
          handle_operands(e, true);
          return action::keep;
        }
      }

      template<OneOf<e::type_assert, e::dyn_type_assert, e::call,
      e::full_call, e::act_rec_wr, e::sym_val_wr, e::range_set> E>
      action operator()(E * J_NOT_NULL e, bool) noexcept {
        handle_operands(e, true);
        return action::keep;
      }

      void handle_operands(expr * J_NOT_NULL expr, bool result_used, progn_mode mode = progn_mode::none) noexcept {
        i32_t num_in = expr->num_inputs - 1;
        for (i32_t i = num_in; i >= 0; --i) {
          bool cur_result_used = result_used;
          switch (mode) {
          case progn_mode::none: break;
          case progn_mode::progn: cur_result_used &= i == num_in; break;
          case progn_mode::prog1: cur_result_used &= i == 0; break;
          }
          switch (visit(expr->input(i).expr, cur_result_used)) {
          case action::remove:
            ctx.erase_operand(*expr, i);
            break;
          case action::convert_to_progn:
            ctx.convert_operand_to_progn(*expr, i);
            break;
          case action::keep:
            break;
          }
        }
      }

      bool handle_operand(expr * J_NOT_NULL expr, bool result_used) noexcept {
        switch (visit(expr, result_used)) {
        case action::remove:
          ctx.erase_operand(*expr->parent, expr->parent_pos);
          return true;
        case action::convert_to_progn:
          ctx.convert_operand_to_progn(*expr->parent, expr->parent_pos);
          return false;
        case action::keep:
          return false;
        }
      }

      template<OneOf<e::ineg, e::lnot, e::to_bool,
      e::eq, e::neq,
      e::tag, e::untag, e::as_range, e::as_vec,
      e::iadd, e::imul, e::isub, e::idiv,
      e::make_vec, e::range_copy, e::vec_append, e::vec, e::range_length, e::range_get,
      e::vec_build> E>
      action operator()(E * J_NOT_NULL e, bool result_used) noexcept {
        handle_operands(e, result_used);
        return result_used ? action::keep : (e->num_inputs ? action::convert_to_progn : action::remove);
      }

      action operator()(e::b_if * e, bool result_used) noexcept {
        auto then_result = handle_operand(e->input(1).expr, result_used);
        auto else_result = handle_operand(e->input(2).expr, result_used);
        if (handle_operand(e->input(0).expr, !then_result || !else_result)) {
          return (then_result && else_result) ? action::remove : action::convert_to_progn;
        }
        if (!e->input(1).expr) {
          ctx.create_nil(*e, 1);
        }
        if (!e->input(2).expr) {
          ctx.create_nil(*e, 2);
        }
        return action::keep;
      }

      template<OneOf<e::lor, e::land> E>
      action operator()(E * J_NOT_NULL e, bool result_used) noexcept {
        if (e->num_inputs) {
          auto ins = e->inputs();
          auto & last_in = ins.pop_back();
          for (auto & in : ins) {
            handle_operand(in.expr, true);
          }
          handle_operand(last_in.expr, result_used);
        }
        if (!e->num_inputs) {
          return result_used ? action::keep : action::remove;
        }
        return e->num_inputs == 1 ? action::convert_to_progn : action::keep;
      }

      action operator()(e::closure * J_NOT_NULL e, bool result_used) noexcept {
        if (!result_used) {
          return action::remove;
        }
        visit(e->input(1).expr, true);
        return action::keep;
      }

      template<OneOf<e::progn, e::let> E>
      action operator()(E * J_NOT_NULL e, bool result_used) noexcept {
        handle_operands(e, result_used, progn_mode::progn);
        return e->num_inputs ? action::keep : action::remove;
      }

      action operator()(e::prog1 * J_NOT_NULL e, bool result_used) noexcept {
        handle_operands(e, result_used, progn_mode::prog1);
        return e->num_inputs ? action::keep : action::remove;
      }

      action visit(expr * J_NOT_NULL e, bool result_used) noexcept {
        return exprs::visit(*this, e, result_used);
      }
    };

    void air_dce(air_pass_context & ctx) {
      u32_t var_uses_1[ctx.fn->num_lex_vars], var_uses_2[ctx.fn->num_lex_vars];
      ::j::memzero(var_uses_1, ctx.fn->num_lex_vars * sizeof(u32_t));
      dce_visitor v{
        .ctx{ctx.pool, *ctx.fn},
        .var_uses_wr = var_uses_1,
      };
      v.visit(ctx.fn->body, true);

      bool did_find = false;
      for (u32_t i = 0; i < ctx.fn->num_lex_vars; ++i) {
        if (var_uses_1[i] == 0) {
          did_find = true;
          break;
        }
      }
      if (!did_find) {
        return;
      }
      u32_t * rd = var_uses_1;
      u32_t * wr = var_uses_2;
      do {
        ::j::memzero(wr, ctx.fn->num_lex_vars * sizeof(u32_t));
        v.did_remove_var = false;
        v.var_uses_rd = rd;
        v.var_uses_wr = wr;

        v.visit(ctx.fn->body, true);

        u32_t * tmp = rd;
        rd = wr;
        wr = tmp;
      } while (v.did_remove_var);
    }
  }

  const air_pass dce_pass{air_dce, "dce", "Dead code elimination", true};
}
