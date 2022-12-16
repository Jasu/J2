#pragma once

#include "lisp/air/exprs/expr_type.hpp"

namespace j::lisp::air::exprs {
  struct expr_counts final {
    u16_t expr_counts[num_expr_types_v] = { 0U };

    J_A(AI) inline void add_expr(expr_type t) noexcept {
      ++expr_counts[(u8_t)t];
    }

    J_A(AI) inline void remove_expr(expr_type t) noexcept {
      --expr_counts[(u8_t)t];
    }

    J_A(AI,NODISC) inline u16_t operator[](expr_type t) const noexcept {
      return expr_counts[(u8_t)t];
    }

    J_A(AI,NODISC) inline u16_t operator[](u8_t i) const noexcept {
      return expr_counts[i];
    }

    void reset() noexcept {
      ::j::memzero(expr_counts, num_expr_types_v * sizeof(u16_t));
    }
  };
}
