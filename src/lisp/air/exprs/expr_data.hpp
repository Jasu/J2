#pragma once

#include "lisp/air/exprs/expr_type.hpp"
#include "lisp/air/exprs/expr_flags.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::air::exprs {
  struct input_data final {
    const char * name = nullptr;
  };

  struct const_fold_data final {
    u8_t num_args = 0U;
    expr_type tail_fold_type = expr_type::none;
  };

  struct expr_data final {
    const char * inputs[6] = { nullptr };
    const char * statics[2] = { nullptr };
    expr_flags flags = {};
    lisp_imm identity_element = {};
    lisp_imm absorbing_element = {};
    lisp_imm nullary_result = {};
    lisp_imm unary_result = {};
    const_fold_data fold = {};

    const char * get_input_name(u8_t i) const noexcept {
      return inputs[min(i, 5)];
    }

    const char * get_static_name(u8_t i) const noexcept {
      return statics[min(i, 1)];
    }
  };

  struct expr_data_table final {
    expr_data data[num_expr_types_with_none_v];

    const expr_data & operator[](expr_type t) const noexcept {
      return this->data[(u8_t)t];
    }

    [[nodiscard]] J_RETURNS_NONNULL const char * get_input_name(expr_type t, u8_t i) const noexcept {
      return this->data[(u8_t)t].inputs[min(i, 5)];
    }

    J_INLINE_GETTER_NONNULL constexpr const expr_data * begin() const noexcept {
      return data;
    }

    J_INLINE_GETTER_NONNULL constexpr const expr_data * end() const noexcept {
      return data + num_expr_types_v;
    }
  };

  extern const expr_data_table expr_data_v;
}
