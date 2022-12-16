#pragma once

#include "lisp/values/rep.hpp"
#include "hzd/crc32.hpp"

namespace j::lisp::air::inline values {
  struct imm_type_mask_hash final {
    J_A(NODISC,FLATTEN) u32_t operator()(imm_type_mask spec) const noexcept {
      return crc32(117, (u32_t)spec.value());
    }
  };

  struct rep_spec_hash final {
    J_A(NODISC,FLATTEN) u32_t operator()(rep_spec spec) const noexcept {
      return crc32(119, (u32_t)spec.value);
    }
  };
}
