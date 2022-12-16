#pragma once

#include "lisp/air/values/val_spec.hpp"
#include "containers/span.hpp"

namespace j::lisp::air::exprs {
  struct expr;

  struct input final {
    expr * expr = nullptr;
    val_spec type;

    J_INLINE_GETTER bool empty() const noexcept {
      return !expr;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !expr;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return expr;
    }
  };

  using input_span = span<input>;
  using const_input_span = span<const input>;
}
