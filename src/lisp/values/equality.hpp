#pragma once

#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::inline values {
  J_INLINE_GETTER bool eq(lisp_imm lhs, lisp_imm rhs) noexcept {
    return lhs.raw == rhs.raw;
  }

  [[nodiscard]] bool shallow_vec_eq(lisp_imm lhs, lisp_imm rhs) noexcept;
}
