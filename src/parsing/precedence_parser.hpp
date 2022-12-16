#pragma once

#include "exceptions/assert.hpp"

namespace j::parsing {
  enum operator_type : u8_t {
    infix,
    prefix,
    postfix,
  };

  struct operator_info {
    u8_t precedence = 0;
    operator_type type = infix;
    const char * name = nullptr;
  };

  namespace detail {
    J_A(NORET,NI) inline void throw_unexpected_op_type(const operator_info * J_NOT_NULL info) {
      const char * tname = info->type == infix ? "infix" : info->type == prefix ? "prefix" : "postfix";
      J_THROW("Unexpected {} operator {}", tname, info->name);
    }
  }

  constexpr inline i32_t op_stack_size = 16;
  constexpr inline i32_t expr_stack_size = 16;

  template<typename Expr, typename Op, auto GetOpInfo, auto ReduceBinary, auto ReduceUnary>
  struct precedence_parser {
    using op_ref_t = conditional_t<is_scalar_v<Op>, Op, Op &&>;
    using expr_ref_t = conditional_t<is_scalar_v<Expr>, Expr, Expr &&>;
    struct J_AT(HIDDEN) op_entry final {
      const operator_info * info = nullptr;
      Op op{};
    };
    op_entry * cur_op;
    Expr * cur_expr;
    bool prev_was_expr_or_suffix = false;
    op_entry op_stack[op_stack_size];
    Expr expr_stack[expr_stack_size];

    J_A(AI,ND) inline precedence_parser() noexcept
      : cur_op(op_stack),
        cur_expr(expr_stack)
    {
    }

    J_A(AI,ND,NODISC) inline bool accepts_prefix() const noexcept {
      return !prev_was_expr_or_suffix;
    }

    J_A(AI,ND,NODISC) inline bool accepts_postfix() const noexcept {
      return prev_was_expr_or_suffix;
    }

    J_A(AI,ND,NODISC) inline bool accepts_infix() const noexcept {
      return prev_was_expr_or_suffix;
    }

    J_A(AI,ND,NODISC) inline bool accepts_expr() const noexcept {
      return !prev_was_expr_or_suffix;
    }

    void push_op(op_ref_t op) {
      const operator_info * new_op_info = GetOpInfo(op);
      if (J_UNLIKELY(prev_was_expr_or_suffix != (new_op_info->type != prefix))) {
        detail::throw_unexpected_op_type(new_op_info);
      }
      if (new_op_info->type != prefix) {
        reduce_at_or_above(new_op_info->precedence);
        if (new_op_info->type == postfix) {
          *cur_expr = ReduceUnary((op_ref_t)(op), (expr_ref_t)*cur_expr);
          return;
        }
        prev_was_expr_or_suffix = false;
      }
      *++cur_op = op_entry{new_op_info, static_cast<op_ref_t>(op)};
    }

    J_A(NODISC) expr_ref_t finish() {
      push_right_paren();
      return (expr_ref_t)expr_stack[1];
    }

    void push_expr(expr_ref_t e) {
      J_REQUIRE(!prev_was_expr_or_suffix, "Unexpected expression");
      prev_was_expr_or_suffix = true;
      *++cur_expr = (expr_ref_t)e;
    }

    void reduce_top() noexcept {
      if (cur_op->info->type == infix) {
        Expr * rhs = cur_expr--;
        *cur_expr = ReduceBinary((op_ref_t)((cur_op--)->op), (expr_ref_t)*cur_expr, (expr_ref_t)*rhs);
      } else {
        *cur_expr = ReduceUnary((op_ref_t)((cur_op--)->op), (expr_ref_t)*cur_expr);
      }
    }

    void push_left_paren() {
      J_REQUIRE(!prev_was_expr_or_suffix, "Unexpected left paren");
      *++cur_op = {};
    }

    void push_right_paren() {
      J_REQUIRE(prev_was_expr_or_suffix, "Unexpected right paren");
      while (cur_op->info) {
        reduce_top();
      }
      --cur_op;
    }

    void reduce_at_or_above(u8_t precedence) {
      while (cur_op->info && cur_op->info->precedence >= precedence) {
        reduce_top();
      }
    }
  };
}
