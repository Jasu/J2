#pragma once

#include "hzd/utility.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::lisp::air::analysis {
  struct def_use_set final {
    u32_t num_uses = 0U;
    u32_t num_defs = 0U;

    inline span<exprs::expr *> defs() const noexcept {
      return { (exprs::expr**)(this + 1U), num_defs };
    }

    inline span<exprs::expr *> uses() const noexcept {
      return { (exprs::expr**)(this + 1U) + num_defs, num_uses };
    }
  };
}
