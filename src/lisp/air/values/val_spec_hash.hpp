#pragma once

#include "hzd/crc32.hpp"
#include "lisp/air/values/val_spec.hpp"

namespace j::lisp::air::inline values {
  struct val_spec_hash final {
    J_A(NODISC,FLATTEN) u32_t operator()(val_spec spec) const noexcept {
      return crc32((spec.types.value() << 7) + 11, (u32_t)spec.reps.value);
    }
  };
}
