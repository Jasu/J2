#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::lisp::assembly::amd64 {
  namespace vex {
    /// 2-byte VEX prefix header
    constexpr inline u32_t vex_prefix_2 = 0b11000101U;
    /// 3-byte VEX prefix header
    constexpr inline u32_t vex_prefix_3 = 0b11000100U;

    enum class r_t : u32_t {
      r0 = J_BIT(15),
      r1 = 0U,
    };

    constexpr inline r_t r0 = r_t::r0;
    constexpr inline r_t r1 = r_t::r1;

    /// X-bit in a 3-byte VEX prefix
    enum class x_t : u32_t {
      x0 = J_BIT(14),
      x1 = 0U,
    };

    constexpr inline x_t x0 = x_t::x0;
    constexpr inline x_t x1 = x_t::x1;

    /// B-bit in a 3-byte VEX prefix
    enum class b_t : u32_t {
      b0 = J_BIT(13),
      b1 = 0U,
    };

    constexpr inline b_t b0 = b_t::b0;
    constexpr inline b_t b1 = b_t::b1;

    enum class mm_t : u32_t {
      mm_0f = 0b01U << 8,
      mm_0f38 = 0b10U << 8,
      mm_0f3a = 0b11U << 8,
    };

    constexpr inline mm_t mm_0f = mm_t::mm_0f;
    constexpr inline mm_t mm_0f38 = mm_t::mm_0f38;
    constexpr inline mm_t mm_0f3a = mm_t::mm_0f3a;

    enum class w_t : u32_t {
      w0 = 0U,
      w1 = J_BIT(23),
    };
    constexpr inline w_t w0 = w_t::w0;
    constexpr inline w_t w1 = w_t::w1;

    enum class l_t : u32_t {
      l0 = 0U,
      l1 = J_BIT(10),
    };

    constexpr inline l_t l0 = l_t::l0;
    constexpr inline l_t l1 = l_t::l1;

    enum class pp_t : u32_t {
      pp_66 = 0b01U << 8,
      pp_f3 = 0b10U << 8,
      pp_f2 = 0b11U << 8,
    };

    constexpr inline pp_t pp_66 = pp_t::pp_66;
    constexpr inline pp_t pp_f3 = pp_t::pp_f3;
    constexpr inline pp_t pp_f2 = pp_t::pp_f2;

    J_INLINE_GETTER constexpr bool can_enc_short(r_t)     noexcept { return true;        }
    J_INLINE_GETTER constexpr bool can_enc_short(pp_t)    noexcept { return true;        }
    J_INLINE_GETTER constexpr bool can_enc_short(l_t)     noexcept { return true;        }
    J_INLINE_GETTER constexpr bool can_enc_short(x_t x)   noexcept { return x  == x0;    }
    J_INLINE_GETTER constexpr bool can_enc_short(b_t b)   noexcept { return b  == b0;    }
    J_INLINE_GETTER constexpr bool can_enc_short(mm_t mm) noexcept { return mm == mm_0f; }
    J_INLINE_GETTER constexpr bool can_enc_short(w_t w)   noexcept { return w  == w0;    }

    template<typename... T>
    J_INLINE_GETTER constexpr bool can_encode_short(T... parts) noexcept {
      return (can_enc_short(parts) && ...);
    }

    J_INLINE_GETTER constexpr u32_t enc_short(r_t r)   noexcept { return (u32_t)r;   }
    J_INLINE_GETTER constexpr u32_t enc_short(pp_t pp) noexcept { return (u32_t)pp;  }
    J_INLINE_GETTER constexpr u32_t enc_short(l_t l)   noexcept { return (u32_t)l;   }
    J_INLINE_GETTER constexpr u32_t enc_short(x_t x)   noexcept { J_ASSERT(x == x1); return 0U; }
    J_INLINE_GETTER constexpr u32_t enc_short(b_t b)   noexcept { J_ASSERT(b == b1); return 0U; }
    J_INLINE_GETTER constexpr u32_t enc_short(mm_t mm) noexcept { J_ASSERT(mm == mm_0f); return 0U; }
    J_INLINE_GETTER constexpr u32_t enc_short(w_t w)   noexcept { J_ASSERT(w == w0); return 0U; }

    J_INLINE_GETTER constexpr u32_t enc_long(r_t r)   noexcept { return (u32_t)r;   }
    J_INLINE_GETTER constexpr u32_t enc_long(pp_t pp) noexcept { return (u32_t)pp << 8;  }
    J_INLINE_GETTER constexpr u32_t enc_long(l_t l)   noexcept { return (u32_t)l << 8;   }
    J_INLINE_GETTER constexpr u32_t enc_long(x_t x)   noexcept { return (u32_t)x; }
    J_INLINE_GETTER constexpr u32_t enc_long(b_t b)   noexcept { return (u32_t)b; }
    J_INLINE_GETTER constexpr u32_t enc_long(mm_t mm) noexcept { return (u32_t)mm; }
    J_INLINE_GETTER constexpr u32_t enc_long(w_t w)   noexcept { return (u32_t)w; }

    template<typename... T>
    J_INLINE_GETTER constexpr u32_t encode_long(T ... parts) {
      u32_t result = vex_prefix_3 | (enc_long(parts) | ...);
      if constexpr (!(is_same_v<T, mm_t> || ...)) {
        result |= (u32_t)mm_0f;
      }
      if constexpr (!(is_same_v<T, x_t> || ...)) {
        result |= (u32_t)x1;
      }
      if constexpr (!(is_same_v<T, b_t> || ...)) {
        result |= (u32_t)b1;
      }
      return result;
    }

    template<typename... T>
    J_INLINE_GETTER constexpr u32_t encode_short(T ... parts) {
      return vex_prefix_2 | (enc_short(parts) | ...);
    }

    template<typename... T>
    J_INLINE_GETTER constexpr u32_t encode(T ... parts) {
      if (can_encode_short(parts...)) {
        return encode_short(parts...);
      } else {
        return encode_long(parts...);
      }
    }
  }
}
