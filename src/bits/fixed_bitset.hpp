#pragma once

#include "bits/bitops.hpp"
#include "hzd/mem.hpp"

namespace j::bits {
  namespace detail {
    J_A(ND,NODISC) inline bool fb_empty(const u64_t * J_NOT_NULL bitmask, u32_t num_words) noexcept {
      do {
        if (*bitmask++) {
          return false;
        }
      } while (--num_words);
      return true;
    }

    J_A(ND,NODISC,NI) inline u32_t fb_size(const u64_t * J_NOT_NULL bitmask, u32_t num_words) noexcept {
      u32_t result = 0U;
      do {
        result += popcount(*bitmask++);
      } while (--num_words);
      return result;
    }
  }

  struct fixed_bitset_iterator final {
    u32_t idx = 0U;
    u32_t max = 0U;
    const u64_t *it = nullptr;

    inline fixed_bitset_iterator & fast_forward() noexcept {
      while (idx != max) {
        const u32_t cnt = __builtin_ia32_tzcnt_u64((it[idx >> 6] & (u64_t)(-1L << (idx & 63))));
        idx = (idx & -64) + cnt;
        if (cnt != 64) {
          break;
        }
      }
      return *this;
    }

    J_A(ND,AI,NODISC) u64_t operator*() const noexcept {
      return idx;
    }

    J_A(AI) inline fixed_bitset_iterator & operator++() noexcept {
      return ++idx, fast_forward();
    }

    fixed_bitset_iterator operator++(int) noexcept {
      fixed_bitset_iterator res{*this};
      ++idx;
      fast_forward();
      return res;
    }

    J_A(AI,ND,NODISC) inline bool operator==(const fixed_bitset_iterator & rhs) const noexcept {
      return idx == rhs.idx;
    }
  };

  template<u32_t NumWords>
  struct fixed_bitset final {
    u64_t bitmask[NumWords] = {0};

    J_A(ND,AI) inline void add(u32_t idx) noexcept {
      bitmask[idx >> 6] |= (1UL << (idx & 63));
    }

    inline void del(u32_t idx) noexcept {
      bitmask[idx >> 6] &= ~(1UL << (idx & 63));
    }

    J_A(ND,AI,NODISC) inline bool has(u32_t idx) const noexcept {
      return bitmask[idx >> 6] & (1UL << (idx & 63));
    }

    [[nodiscard]] bool is_subset_of(const fixed_bitset & rhs) const noexcept {
      for (u32_t i = 0U; i < NumWords; ++i) {
        if (bitmask[i] & ~rhs.bitmask[i]) {
          return false;
        }
      }
      return true;
    }

    J_A(ND,NODISC) inline fixed_bitset operator|(const fixed_bitset & rhs) const noexcept;
    J_A(ND) inline fixed_bitset & operator|=(const fixed_bitset & rhs) noexcept;

    J_A(ND,NODISC) inline fixed_bitset operator^(const fixed_bitset & rhs) const noexcept;
    J_A(ND) inline fixed_bitset & operator^=(const fixed_bitset & rhs) noexcept;

    J_A(ND,NODISC) inline fixed_bitset operator&(const fixed_bitset & rhs) const noexcept;
    J_A(ND) inline fixed_bitset & operator&=(const fixed_bitset & rhs) noexcept;

    J_A(ND,NODISC) inline fixed_bitset operator-(const fixed_bitset & rhs) const noexcept;
    J_A(ND) inline fixed_bitset & operator-=(const fixed_bitset & rhs) noexcept;

    J_A(ND) inline void clear() noexcept;
    J_A(ND) inline void fill(u32_t num) noexcept;

    J_A(NODISC) inline explicit operator bool() const noexcept;
    J_A(NODISC) inline bool operator !() const noexcept;

    J_A(NODISC,ND) inline u32_t size() const noexcept { return detail::fb_size(bitmask, NumWords); }

    J_A(NODISC,ND) static inline u32_t capacity() noexcept { return NumWords << 6U; }

    J_A(NODISC,ND) inline bool operator==(const fixed_bitset & rhs) const noexcept = default;

    J_A(NODISC,ND) inline fixed_bitset_iterator begin() const noexcept {
      return fixed_bitset_iterator{0U, NumWords << 6U, bitmask}.fast_forward();
    }

    J_A(NODISC,ND,AI) inline fixed_bitset_iterator end() const noexcept {
      return fixed_bitset_iterator{NumWords << 6U};
    }
  };

  template<u32_t N> J_A(ND,NODISC) inline fixed_bitset<N>::operator bool() const noexcept { return !detail::fb_empty(bitmask, N); }
  template<u32_t N> J_A(ND,NODISC) inline bool fixed_bitset<N>::operator!() const noexcept { return detail::fb_empty(bitmask, N); }

  template<> J_A(ND,AI,NODISC) inline fixed_bitset<1>::operator bool() const noexcept { return bitmask[0]; }
  template<> J_A(ND,AI,NODISC) inline bool fixed_bitset<1>::operator!() const noexcept { return !bitmask[0]; }

  template<> J_A(ND,AI,NODISC) inline fixed_bitset<2>::operator bool() const noexcept { return bitmask[0] || bitmask[1]; }
  template<> J_A(ND,AI,NODISC) inline bool fixed_bitset<2>::operator!() const noexcept { return !bitmask[0] && !bitmask[1]; }

  // template<> J_A(ND,NODISC) inline fixed_bitset<3>::operator bool() const noexcept { return bitmask[0] || bitmask[1] || bitmask[2]; }
  // template<> J_A(ND,NODISC) inline bool fixed_bitset<3>::operator!() const noexcept { return !bitmask[0] && !bitmask[1] && !bitmask[2]; }

  template<> J_A(ND,AI,NODISC) inline fixed_bitset<4>::operator bool() const noexcept { return bitmask[0] || bitmask[1] || bitmask[2] || bitmask[3]; }
  template<> J_A(ND,AI,NODISC) inline bool fixed_bitset<4>::operator!() const noexcept { return !bitmask[0] && !bitmask[1] && !bitmask[2] && !bitmask[3]; }

  template<>
  J_A(ND,AI,HIDDEN) inline void fixed_bitset<1>::clear() noexcept {
    bitmask[0] = 0;
  }

  template<>
  J_A(ND,AI,HIDDEN) inline void fixed_bitset<2>::clear() noexcept {
    bitmask[0] = 0;
    bitmask[1] = 0;
  }
  // template<>
  // J_A(ND,AI,HIDDEN) inline void fixed_bitset<3>::clear() noexcept {
  //   bitmask[0] = 0;
  //   bitmask[1] = 0;
  //   bitmask[2] = 0;
  // }

  template<>
  J_A(ND,AI,HIDDEN) inline void fixed_bitset<4>::clear() noexcept {
    bitmask[0] = 0;
    bitmask[1] = 0;
    bitmask[2] = 0;
    bitmask[3] = 0;
  }

  template<>
  J_A(ND,AI) inline void fixed_bitset<1>::fill(u32_t num) noexcept {
    bitmask[0] = num != 64U ? ~0UL + (1UL << (num & 63)) : ~0UL;
  }

  template<u32_t N>
  J_A(ND,AI) inline void fixed_bitset<N>::fill(u32_t num) noexcept {
    u64_t mask = (num & 63) ? ~0UL + (1UL << (num & 63)) : 0UL;
    num >>= 6;
    for (u32_t i = 0U; i < N; ++i) {
      if (i < num) {
        bitmask[i] = ~0UL;
      } else if (i == num) {
        bitmask[i] = mask;
      } else {
        bitmask[i] = 0;
      }
    }
  }

  template<u32_t N>
  J_A(ND) inline void fixed_bitset<N>::clear() noexcept {
    ::j::memzero(bitmask, sizeof(bitmask));
  }

#define J_DEFINE_FB_OP(NAME, OP, ...)                                       \
  template<u32_t N> J_A(ND,NODISC,NI) inline fixed_bitset<N> fixed_bitset<N>::operator NAME(const fixed_bitset & rhs) const noexcept { \
    fixed_bitset result; \
    for (u32_t i = 0U; i < N; ++i) { \
      result.bitmask[i] = bitmask[i] OP __VA_ARGS__ rhs.bitmask[i]; \
    } \
    return result; \
  } \
  template<> J_A(ND,NODISC,AI) inline fixed_bitset<1> fixed_bitset<1>::operator NAME(const fixed_bitset & rhs) const noexcept { \
    return fixed_bitset<1>{{bitmask[0] OP __VA_ARGS__ rhs.bitmask[0]}}; \
  } \
  template<> J_A(ND,NODISC,AI) inline fixed_bitset<2> fixed_bitset<2>::operator NAME(const fixed_bitset & rhs) const noexcept { \
    return fixed_bitset<2>{{bitmask[0] OP __VA_ARGS__ rhs.bitmask[0], bitmask[1] OP __VA_ARGS__ rhs.bitmask[1]}}; \
  } \
  template<> J_A(ND,NODISC) inline fixed_bitset<4> fixed_bitset<4>::operator NAME(const fixed_bitset & rhs) const noexcept { \
    return fixed_bitset<4>{{bitmask[0] OP __VA_ARGS__ rhs.bitmask[0], bitmask[1] OP __VA_ARGS__ rhs.bitmask[1], bitmask[2] OP __VA_ARGS__ rhs.bitmask[2], bitmask[3] OP __VA_ARGS__ rhs.bitmask[3]}}; \
  } \
  template<u32_t N> J_A(ND,NI) inline fixed_bitset<N> & fixed_bitset<N>::operator NAME##=(const fixed_bitset & rhs) noexcept { \
    for (u32_t i = 0U; i < N; ++i) { \
      bitmask[i] OP##= __VA_ARGS__ rhs.bitmask[i]; \
    } \
    return *this; \
  } \
  template<> J_A(ND,AI) inline fixed_bitset<1> & fixed_bitset<1>::operator NAME##=(const fixed_bitset & rhs) noexcept { \
    bitmask[0] OP##= __VA_ARGS__ rhs.bitmask[0];                        \
    return *this; \
  } \
  template<> J_A(ND,AI) inline fixed_bitset<2> & fixed_bitset<2>::operator NAME##=(const fixed_bitset & rhs) noexcept { \
    bitmask[0] OP##= __VA_ARGS__ rhs.bitmask[0];                        \
    bitmask[1] OP##= __VA_ARGS__ rhs.bitmask[1];                        \
    return *this; \
  } \
  template<> J_A(ND) inline fixed_bitset<4> & fixed_bitset<4>::operator NAME##=(const fixed_bitset & rhs) noexcept { \
    bitmask[0] OP##= __VA_ARGS__ rhs.bitmask[0];                        \
    bitmask[1] OP##= __VA_ARGS__ rhs.bitmask[1];                        \
    bitmask[2] OP##= __VA_ARGS__ rhs.bitmask[2];                        \
    bitmask[3] OP##= __VA_ARGS__ rhs.bitmask[3];                        \
    return *this; \
  }

  // template<> J_A(ND,NODISC) inline fixed_bitset<3> fixed_bitset<3>::operator NAME(const fixed_bitset & rhs) const noexcept { \
  //   return fixed_bitset<3>{{bitmask[0] OP __VA_ARGS__ rhs.bitmask[0], bitmask[1] OP __VA_ARGS__ rhs.bitmask[1], bitmask[2] OP __VA_ARGS__ rhs.bitmask[2]}}; \
  // } \
  //
  // template<> J_A(ND) inline fixed_bitset<3> & fixed_bitset<3>::operator NAME##=(const fixed_bitset & rhs) noexcept { \
  //   bitmask[0] OP##= __VA_ARGS__ rhs.bitmask[0];                        \
  //   bitmask[1] OP##= __VA_ARGS__ rhs.bitmask[1];                        \
  //   bitmask[2] OP##= __VA_ARGS__ rhs.bitmask[2];                        \
  //   return *this; \
  // } \
  //

  J_DEFINE_FB_OP(|, |)
  J_DEFINE_FB_OP(&, &)
  J_DEFINE_FB_OP(-, &, ~)
  J_DEFINE_FB_OP(^, ^)

  using bitset64  = fixed_bitset<1>;
  using bitset128 = fixed_bitset<2>;
  using bitset256 = fixed_bitset<4>;
}
