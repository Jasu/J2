#pragma once

#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/air/exprs/expr_flags.hpp"
#include "lisp/air/exprs/expr_data.hpp"
#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::air::exprs {
  struct expr_criterion_expr_type final {
    expr_type types[4] = { expr_type::none, expr_type::none, expr_type::none, expr_type::none };

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      return types[0] == expr_type::none || e->type == types[0]
        || e->type == types[1] || e->type == types[2] || e->type == types[3];
    }
  };

  struct expr_criterion_parent_expr_type final {
    expr_type types[4] = { expr_type::none, expr_type::none, expr_type::none, expr_type::none };

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      return types[0] == expr_type::none
        || (e->parent &&
            (e->parent->type == types[0] || e->parent->type == types[1]
             || e->parent->type == types[2] || e->parent->type == types[3]));
    }
  };


  struct expr_criterion_input_expr_type final {
    i8_t child_index = 0;
    expr_type type = expr_type::none;

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      u8_t idx = child_index < 0 ? e->num_inputs + child_index : child_index;
      return type == expr_type::none || (e->num_inputs > idx && e->input(idx).expr->type == type);
    }
  };

   inline expr_criterion_expr_type is(expr_type t0,
                                               expr_type t1 = expr_type::none,
                                               expr_type t2 = expr_type::none,
                                               expr_type t3 = expr_type::none) noexcept {
    return {{ t0, t1, t2, t3 }};
  }

   inline expr_criterion_parent_expr_type parent_is(expr_type t0,
                                                             expr_type t1 = expr_type::none,
                                                             expr_type t2 = expr_type::none,
                                                             expr_type t3 = expr_type::none) noexcept {
    return {{ t0, t1, t2, t3 }};
  }

   inline expr_criterion_input_expr_type nth_input_is(i8_t n, expr_type t) noexcept {
    return { n, t };
  }

   inline expr_criterion_input_expr_type first_input_is(expr_type t) noexcept {
    return { 0, t };
  }

   inline expr_criterion_input_expr_type last_input_is(expr_type t) noexcept {
    return { -1, t };
  }

  struct expr_criterion_truthiness final {
    truthiness_condition cond = truthiness_condition::any;

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      if (cond == truthiness_condition::any) {
        return true;
      }
      truthiness_condition cur_cond = cond;

      do {
        if (is_progn_like(e->type) || e->type == expr_type::to_bool) {
          if (!e->num_inputs) {
            return cur_cond & false;
          }
          e = e->input(e->num_inputs - 1).expr;
          continue;
        }

        if (e->type == expr_type::lnot) {
          cur_cond = flip_condition(cur_cond);
          e = e->input(0).expr;
          continue;
        }
      } while (false);

      if (e->type == expr_type::ld_const) {
        return cond & (bool)*(const lisp_imm *)e->static_begin();
      }
      return cond & type_truthiness(e->result.types);
    }
  };

  constexpr inline expr_criterion_truthiness must_be_truthy{truthiness_condition::must_be_truthy};
  constexpr inline expr_criterion_truthiness must_be_falsy{truthiness_condition::must_be_falsy};
  constexpr inline expr_criterion_truthiness may_be_truthy{truthiness_condition::may_be_truthy};
  constexpr inline expr_criterion_truthiness may_be_falsy{truthiness_condition::may_be_falsy};

  enum class comparison_op : u8_t {
    none = 0U,

    eq,
    neq,

    lt,
    lte,
    gt,
    gte,
  };

  struct expr_criterion_const final {
    comparison_op op = comparison_op::none;
    lisp_imm const_val{};

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      if (op == comparison_op::none) {
        return true;
      }

      const lisp_imm * val = nullptr;
      if (e->type == expr_type::ld_const) {
        val = reinterpret_cast<const lisp_imm*>(e->static_begin());
      } else if (e->type == expr_type::nop) {
        val = &lisp_nil_v;
      } else {
        return false;
      }

      switch (op) {
      case comparison_op::none:
        J_UNREACHABLE();
      case comparison_op::eq:
        return val->eq(const_val);
      case comparison_op::neq:
        return !val->eq(const_val);
      case comparison_op::lt:
        return val->is_i64() && (i64_t)val->raw < (i64_t)const_val.raw;
      case comparison_op::lte:
        return val->is_i64() && (i64_t)val->raw <= (i64_t)const_val.raw;
      case comparison_op::gt:
        return val->is_i64() && (i64_t)val->raw > (i64_t)const_val.raw;
      case comparison_op::gte:
        return val->is_i64() && (i64_t)val->raw >= (i64_t)const_val.raw;
      }
    }
  };

   inline expr_criterion_const const_eq(const lisp_imm & imm) noexcept {
    return { comparison_op::eq, imm };
  }

   inline expr_criterion_const const_eq(i64_t val) noexcept {
    return { comparison_op::eq, lisp_i64(val) };
  }

   inline expr_criterion_const const_neq(const lisp_imm & imm) noexcept {
    return { comparison_op::neq, imm };
  }

   inline expr_criterion_const const_neq(i64_t val) noexcept {
    return { comparison_op::neq, lisp_i64(val) };
  }

   inline expr_criterion_const const_lt(i64_t val) noexcept {
    return { comparison_op::lt, lisp_i64(val) };
  }

   inline expr_criterion_const const_lte(i64_t val) noexcept {
    return { comparison_op::lte, lisp_i64(val) };
  }

   inline expr_criterion_const const_gt(i64_t val) noexcept {
    return { comparison_op::gt, lisp_i64(val) };
  }

   inline expr_criterion_const const_gte(i64_t val) noexcept {
    return { comparison_op::gte, lisp_i64(val) };
  }

  const inline expr_criterion_const is_nil = { comparison_op::eq, lisp_nil_v };
  const inline expr_criterion_const is_false = { comparison_op::eq, lisp_false_v };
  const inline expr_criterion_const is_true = { comparison_op::eq, lisp_true_v };


  struct expr_criterion_input_count final {
    u8_t min_inputs = 0U, max_inputs = 255U;

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      return e->num_inputs >= min_inputs && e->num_inputs <= max_inputs;
    }
  };

   inline expr_criterion_input_count num_inputs_eq(u8_t num) noexcept {
    return {num, num};
  }

   inline expr_criterion_input_count num_inputs_between(u8_t min, u8_t max) noexcept {
    return {min, max};}

   inline expr_criterion_input_count min_inputs(u8_t min) noexcept {
    return {.min_inputs = min};
  }

   inline expr_criterion_input_count max_inputs(u8_t max) noexcept {
    return {.max_inputs = max};
  }

  constexpr inline expr_criterion_input_count has_no_inputs = {.max_inputs = 0U};
  constexpr inline expr_criterion_input_count is_unary = {1U, 1U};
  constexpr inline expr_criterion_input_count is_binary = {2U, 2U};
  constexpr inline expr_criterion_input_count has_inputs = {.min_inputs = 1U};

  struct expr_criterion_input_index final {
    i8_t min = 0, max = -1;

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      if (!e->parent) {
        return min == 0 && max != 0;
      }
      i8_t min_idx = min < 0 ? e->parent->num_inputs + min : min;
      i8_t max_idx = max < 0 ? e->parent->num_inputs + max : max;
      return (i8_t)e->parent_pos >= min_idx && (i8_t)e->parent_pos <= max_idx;
    }
  };

   inline expr_criterion_input_index input_index_eq(i8_t num) noexcept {
    return {num, num};
  }

   inline expr_criterion_input_index input_index_between(i8_t min, i8_t max) noexcept {
    return {min, max};
  }

   inline expr_criterion_input_index min_input_index(i8_t min) noexcept {
    return {.min = min};
  }

   inline expr_criterion_input_index max_input_index(i8_t max) noexcept {
    return {.max = max};
  }

  constexpr inline expr_criterion_input_index is_first_input = {0, 0};
  constexpr inline expr_criterion_input_index is_not_first_input = {1, -1};
  constexpr inline expr_criterion_input_index is_last_input = {-1, -1};
  constexpr inline expr_criterion_input_index is_not_last_input = {0, -2};
  constexpr inline expr_criterion_input_index is_only_input = {-1, 0};

  struct expr_criterion_symbol_value final {
    id symbol{};

    [[nodiscard]] inline bool check(const expr * J_NOT_NULL e) const noexcept {
      return !symbol || (e->type == expr_type::sym_val_rd
                         && *reinterpret_cast<const id*>(e->static_begin()) == symbol);
    }
  };

   inline expr_criterion_symbol_value is_symbol_value(const id & id) noexcept {
    return { id };
  }

  constexpr inline expr_criterion_symbol_value is_empty_vec = { static_ids::id_empty_vec };

  struct expr_criteria final {
    expr_criterion_expr_type type{};
    expr_criterion_parent_expr_type parent_type{};
    expr_criterion_input_expr_type input_type{};
    expr_criterion_input_count input_count{};
    expr_criterion_input_index input_index{};
    expr_criterion_const const_val{};
    expr_criterion_symbol_value symbol_value{};
    expr_criterion_truthiness truthiness{};

    bool check(const expr * J_NOT_NULL e) const noexcept {
      return type.check(e) && parent_type.check(e) && input_type.check(e) && input_count.check(e) && input_index.check(e) && const_val.check(e) && symbol_value.check(e) && truthiness.check(e);
    }

     void set(expr_type t) noexcept {
      for (u8_t i = 0; i < 4; ++i) {
        if (type.types[i] == expr_type::none) {
          type.types[i] = t;
          return;
        }
      }
      J_FAIL("Overflow");
    }

     void set(const expr_criterion_expr_type & t) noexcept {
      type = t;
    }

     void set(const expr_criterion_input_expr_type & t) noexcept {
      input_type = t;
    }

     void set(const expr_criterion_parent_expr_type & t) noexcept {
      parent_type = t;
    }

     void set(const expr_criterion_input_count & c) noexcept {
      input_count = c;
    }

     void set(const expr_criterion_input_index & c) noexcept {
      input_index = c;
    }

     void set(const expr_criterion_const & c) noexcept {
      const_val = c;
    }

     void set(const expr_criterion_symbol_value & c) noexcept {
      symbol_value = c;
    }

     void set(const expr_criterion_truthiness & c) noexcept {
      truthiness = c;
    }
  };
}
