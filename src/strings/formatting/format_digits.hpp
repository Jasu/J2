#pragma once

#include "hzd/utility.hpp"
#include "hzd/mem.hpp"
#include "bits/bitops.hpp"

namespace j::strings::inline formatting {
  J_A(AI,ND,HIDDEN,NODISC) inline constexpr char format_hex_digit(u8_t value, char a_char_minus_ten = 'a' - 10) noexcept {
    return (value < 0xA ? '0' : a_char_minus_ten) + value;
  }

  J_ALWAYS_INLINE_NONNULL char * format_hex2(u8_t value,
                                                char * J_NOT_NULL target,
                                                char a_char_minus_ten = 'a' - 10) noexcept
  {
    *target++ = format_hex_digit(value >> 4, a_char_minus_ten);
    *target++ = format_hex_digit(value & 0xFU, a_char_minus_ten);
    return target;
  }

  J_ALWAYS_INLINE_NONNULL char * format_hex(u64_t value,
                                               char * J_NOT_NULL target,
                                               u8_t num_digits,
                                               char a_char_minus_ten = 'a' - 10) noexcept
  {
    if (J_UNLIKELY(num_digits > 8)) {
      j::memset(target, '0', num_digits - 8);
      target += num_digits - 8;
      num_digits = 8;
    }
    num_digits <<= 2;
    do {
      num_digits -= 4;
      *target++ = format_hex_digit((value >> num_digits) & 0xFU, a_char_minus_ten);
    } while (num_digits);
    return target;
  }

  J_ALWAYS_INLINE_NONNULL char * format_dec(u8_t value, char * J_NOT_NULL target) noexcept {
    if (value >= 10U) {
      if (value >= 100U) {
        *target++ = value >= 200U ? '2' : '1';
        value -= value >= 200U ? 200U : 100U;
      }
      const u8_t digit = value / 10U;
      *target++ = digit + '0';
      value -= digit * 10U;
    }
    *target++ = value + '0';
    return target;
  }

  J_ALWAYS_INLINE void format_dec(u64_t value, char * J_NOT_NULL target, u8_t num_digits) noexcept {
    while (num_digits--) {
      target[num_digits] = '0' + (value % 10U);
      value /= 10U;
    }
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(u8_t v) noexcept {
    return 1U + (u8_t)(v>=10U) + (u8_t)(v>=100U);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(u16_t v) noexcept {
    return 1U + (u8_t)(v>=10U) + (u8_t)(v>=100U) + (u8_t)(v>=1000U) + (u8_t)(v>=10000U);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(u32_t v) noexcept {
    return 1U
      + (u8_t)(v>=10U)
      + (u8_t)(v>=100U)
      + (u8_t)(v>=1000U)
      + (u8_t)(v>=10000U)
      + (u8_t)(v>=100000U)
      + (u8_t)(v>=1000000U)
      + (u8_t)(v>=10000000U)
      + (u8_t)(v>=100000000U)
      + (u8_t)(v>=1000000000U);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(u64_t v) noexcept {
    return 1U
      + (u8_t)(v>=10U)
      + (u8_t)(v>=100U)
      + (u8_t)(v>=1000U)
      + (u8_t)(v>=10000U)
      + (u8_t)(v>=100000U)
      + (u8_t)(v>=1000000U)
      + (u8_t)(v>=10000000U)
      + (u8_t)(v>=100000000U)
      + (u8_t)(v>=1000000000U)
      + (u8_t)(v>=10000000000ULL)
      + (u8_t)(v>=100000000000ULL)
      + (u8_t)(v>=1000000000000ULL)
      + (u8_t)(v>=10000000000000ULL)
      + (u8_t)(v>=100000000000000ULL)
      + (u8_t)(v>=1000000000000000ULL)
      + (u8_t)(v>=10000000000000000ULL)
      + (u8_t)(v>=100000000000000000ULL)
      + (u8_t)(v>=1000000000000000000ULL)
      + (u8_t)(v>=10000000000000000000ULL);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(i8_t v) noexcept {
    return (v < 0)
      ? num_decimal_digits((u16_t)-(i16_t)v) + 1U
      : num_decimal_digits((u8_t)v);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(i16_t v) noexcept {
    return (v < 0)
      ? num_decimal_digits((u32_t)-(i32_t)v) + 1U
      : num_decimal_digits((u16_t)v);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(i32_t v) noexcept {
    return (v < 0)
      ? num_decimal_digits((u64_t)-(i64_t)v) + 1U
      : num_decimal_digits((u32_t)v);
  }

  J_INLINE_GETTER constexpr u8_t num_decimal_digits(i64_t v) noexcept {
    return (v < 0)
      // TODO
      ? num_decimal_digits((u64_t)-v) + 1U
      : num_decimal_digits((u64_t)v);
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(u64_t v) noexcept {
    return 16 - (bits::clz(v | 1UL) >> 2);
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(u32_t v) noexcept {
    return 8 - (bits::clz(v | 1U) >> 2);
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(u16_t v) noexcept {
    return v ? (19U - bits::clz(v)) >> 2U : 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(u8_t v) noexcept {
    return v > 0xF ? 2 : 1;
  }

  J_INLINE_GETTER u8_t num_hex_digits(i64_t v) noexcept {
    return (v < 0) + num_hex_digits((u64_t)__builtin_labs(v));
  }

  J_INLINE_GETTER u8_t num_hex_digits(i32_t v) noexcept {
    return (v < 0) + num_hex_digits((u32_t)__builtin_abs(v));
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(i16_t v) noexcept {
    return (v < 0) ? 1U + num_hex_digits(static_cast<u16_t>(-v)) : num_hex_digits(static_cast<u16_t>(v));
  }

  J_INLINE_GETTER constexpr u8_t num_hex_digits(i8_t v) noexcept {
    return (v < 0) ? 1U + num_hex_digits(static_cast<u8_t>(-v)) : num_hex_digits(static_cast<u8_t>(v));
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(u64_t v) noexcept {
    return v ? 64U - bits::clz(v) : 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(u32_t v) noexcept {
    return v ? 32U - bits::clz(v) : 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(u16_t v) noexcept {
    return v ? 16 - bits::clz(v) : 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(u8_t v) noexcept {
    return v ? 8 - bits::clz(v) : 1U;
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(i64_t v) noexcept {
    return v < 0
      ? 1U + num_binary_digits(static_cast<u64_t>(-v))
      : num_binary_digits(static_cast<u64_t>(v));
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(i32_t v) noexcept {
    return num_binary_digits(static_cast<i64_t>(v));
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(i16_t v) noexcept {
    return num_binary_digits(static_cast<i64_t>(v));
  }

  J_INLINE_GETTER constexpr u8_t num_binary_digits(i8_t v) noexcept {
    return num_binary_digits(static_cast<i64_t>(v));
  }
}
