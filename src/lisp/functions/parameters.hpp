#pragma once

#include "lisp/common/id.hpp"

namespace j::lisp::functions {

  struct alignas(u64_t) parameters final {
    u32_t num_params = 0U;
    u32_t num_optional:30 = 0U;
    bool has_rest:1 = false;
    /// Whether the function is a closure, with an argument reserved as the
    /// static chain pointer.
    ///
    /// \note While SysV ABI specifies `r11` as the static chain pointer, it
    ///       is not used here, to make it directly possible to call closures
    ///       from C++. Instead, the third argument is used (as first two are
    ///       used for passing all other arguments in the `full_call` calling
    ///       convention.
    bool has_static_chain:1 = false;

    J_INLINE_GETTER id operator[](u32_t i) const noexcept {
      J_ASSUME(i < num_params);
      return begin()[i];
    }

    J_INLINE_GETTER_NONNULL const id * begin() const noexcept {
      return reinterpret_cast<const id*>(this + 1);
    }

    J_INLINE_GETTER_NONNULL const id * end() const noexcept {
      return reinterpret_cast<const id*>(this + 1) + num_params;
    }
  };
}
