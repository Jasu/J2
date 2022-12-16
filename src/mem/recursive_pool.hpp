#pragma once

#include "mem/page_pool.hpp"
#include "bits/bitops.hpp"

namespace j::mem {
  namespace detail {
    struct recursive_pool_bitset final {
      u32_t depth:8 = 0U;
      u32_t size:24 = 0U;
      u32_t capacity = 0U;

      [[nodiscard]] J_RETURNS_NONNULL static recursive_pool_bitset * allocate_bitset(u32_t depth);
      [[nodiscard]] J_RETURNS_NONNULL static recursive_pool_bitset * reallocate_bitset(u32_t depth, recursive_pool_bitset * J_NOT_NULL old);

      J_INLINE_GETTER static u32_t allocate(recursive_pool_bitset * J_NOT_NULL & bitset) {
        u64_t * const bits = reinterpret_cast<u64_t*>(bitset + 1U);
        u32_t index = bits::clz_safe(*bits);
        const u8_t d = bitset->depth;
        J_ASSUME(d < 4U);
        if (J_UNLIKELY(index == 64U)) {
          bitset = reallocate_bitset(d + 1, bitset);
          return allocate(bitset);
        }

        u32_t qword_index = 0U;
        switch (d) {
        case 3:
          qword_index += 1U + index;
          index = index * 64U + bits::clz(bits[qword_index]);
        case 2:
          qword_index += 1U + index;
          index = index * 64U + bits::clz(bits[qword_index]);
        case 1:
          qword_index += 1U + index;
          index = index * 64U + bits::clz(bits[qword_index]);
        case 0:
          bits[qword_index] &= ~(1UL << index);
          if (bits[qword_index]) {
            return index;
          }
        default:
          J_UNREACHABLE();
        }


        return index;
      }
    };
  }
}
