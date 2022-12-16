#pragma once

#include "math/fixed.hpp"
#include "hzd/crc32.hpp"

namespace j::math {
  template<typename T>
  struct fixed_hash final {
    J_A(ND, FLATTEN) u32_t operator()(const T & num) const noexcept {
      return crc32(19, static_cast<make_unsigned_t<typename T::inner_type>>((u64_t)num.raw()));
    }
  };
}
