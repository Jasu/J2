#pragma once

#include "bits/bitops.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::bits {
  template<u32_t NumU64>
  class static_bitset final {
  public:
    static constexpr inline u32_t npos = U32_MAX;

    void set_first_n(u32_t count) noexcept {
      J_ASSERT(count <= NumU64 * 64U, "Out of bounds.");
      u64_t * ptr = m_data;
      while (count >= 64U) {
        *ptr++ = U64_MAX;
        count -= 64U;
      }
      if (count) {
        *ptr |= -(1ULL << (64U - count));
      }
    }

    void clear_first_n(u32_t count) noexcept {
      J_ASSERT(count <= NumU64 * 64U, "Out of bounds.");
      u64_t * ptr = m_data;
      while (count >= 64U) {
        *ptr++ = 0U;
        count -= 64U;
      }
      if (count) {
        *ptr &= (1ULL << count) - 1ULL;
      }
    }

    void set_last_n(u32_t count) noexcept {
      J_ASSERT(count <= NumU64 * 64U, "Out of bounds.");
      u64_t * ptr = m_data + NumU64;
      while (count >= 64U) {
        *--ptr = U64_MAX;
        count -= 64U;
      }
      if (count) {
        *--ptr |= (1ULL << (64U - count)) - 1ULL;
      }
    }

    void clear_last_n(u32_t count) noexcept {
      J_ASSERT(count <= NumU64 * 64U, "Out of bounds.");
      u64_t * ptr = m_data + NumU64;
      while (count >= 64U) {
        *--ptr = 0U;
        count -= 64U;
      }
      if (count) {
        *--ptr &= -(1ULL << count);
      }
    }

    explicit operator bool() const noexcept {
      for (u32_t i = 0; i < NumU64; ++i) {
        if (m_data[i]) {
          return true;
        }
      }
      return false;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !operator bool();
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !operator bool();
    }

    bool full() const noexcept {
      for (u32_t i = 0; i < NumU64; ++i) {
        if (m_data[i] != U64_MAX) {
          return false;
        }
      }
      return true;
    }

    J_INLINE_GETTER bool at(u32_t i) const noexcept {
      J_ASSERT(i < NumU64 * 64U);
      return m_data[i >> 6U] & (1ULL << (i & 63U));
    }

    J_ALWAYS_INLINE void set(u32_t i) noexcept {
      J_ASSERT(i < NumU64 * 64U);
      m_data[i >> 6U] |= (1ULL << (i & 63U));
    }


    J_ALWAYS_INLINE void clear(u32_t i) noexcept {
      J_ASSERT(i < NumU64 * 64U);
      m_data[i >> 6U] &= ~(1ULL << (i & 63U));
    }

    void clear() noexcept {
      for (u32_t i = 0U; i < NumU64; ++i) {
        m_data[i] = 0U;
      }
    }

    u32_t find_first_set() const noexcept {
      for (u32_t i = 0U; i < NumU64; ++i) {
        if (const u32_t val = ffs(m_data[i])) {
          return (i << 6U) | val;
        }
      }
      return npos;
    }

    u32_t find_and_clear_first_set() noexcept {
      for (u32_t i = 0U; i < NumU64; ++i) {
        if (m_data[i]) {
          u32_t bit_idx = ctz(m_data[i]);
          m_data[i] -= 1ULL << bit_idx;
          return bit_idx | (i << 6U);
        }
      }
      return npos;
    }

    u32_t find_first_clear() const noexcept {
      for (u32_t i = 0U; i < NumU64; ++i) {
        if (const u64_t v = ~m_data[i]) {
          return ctz(v) | (i << 6U);
        }
      }
      return npos;
    }
  private:
    u64_t m_data[NumU64] = { 0 };
  };
}
