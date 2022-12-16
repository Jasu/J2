#pragma once

#include "hzd/crc32.hpp"
#include "bits/fixed_bitset.hpp"

namespace j::bits {
  template<u32_t N>
  struct fixed_bitset_hash final {
    J_A(ND,NI,FLATTEN) u32_t operator()(const fixed_bitset<N> & bs) const noexcept {
      return crc32(48, (const char*)bs.bitmask, N * 8U);
    }
  };
}
