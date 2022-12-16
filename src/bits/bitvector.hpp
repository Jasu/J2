#pragma once

#include "bits/bitops.hpp"

namespace j::bits {
  struct bitvector {
    union {
      u64_t static_bits;
      u64_t * bits_ptr;
    };

    void set(u32_t size, u32_t index) noexcept {
      u64_t mask = 1U << (index & 63U);
      if (size <= 64U) {
        static_bits |= mask;
      } else {
        bits_ptr[index >> 6U] |= mask;
      }
    }

    void clear(u32_t size, u32_t index) noexcept {
      u64_t mask = ~(1U << (index & 63U));
      if (size <= 64U) {
        static_bits &= mask;
      } else {
        bits_ptr[index >> 6U] &= mask;
      }
    }
  };
}
