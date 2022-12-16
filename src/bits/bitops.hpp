#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/concepts.hpp"

namespace j::bits {
  /// Count trailing zeros (from the least significant bit)
  ///
  /// Result is undefined if [n] is zero.
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(u8_t n) noexcept {
    return __builtin_ctzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(u16_t n) noexcept {
    return __builtin_ctzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(i8_t n) noexcept {
    return __builtin_ctzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(i16_t n) noexcept {
    return __builtin_ctzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(u32_t n) noexcept {
    return __builtin_ctz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(i32_t n) noexcept {
    return __builtin_ctz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(u64_t n) noexcept {
    return __builtin_ctzl(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t ctz(i64_t n) noexcept {
    return __builtin_ctzl(n);
  }

#if J_IS_AMD64
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u16_t ctz_safe(u8_t n) noexcept {
    return __builtin_ctz((u32_t)n | 0x100U);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u16_t ctz_safe(u16_t n) noexcept {
    return __builtin_ctz((u32_t)n | 0x10000U);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u16_t ctz_safe(i8_t n) noexcept {
    return __builtin_ctz((u32_t)n | 0x100U);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u16_t ctz_safe(i16_t n) noexcept {
    return __builtin_ctz((u32_t)n | 0x10000);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u32_t ctz_safe(u32_t n) noexcept {
    return __builtin_ia32_tzcnt_u32(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u32_t ctz_safe(i32_t n) noexcept {
    return __builtin_ia32_tzcnt_u32(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u64_t ctz_safe(u64_t n) noexcept {
    return __builtin_ia32_tzcnt_u64(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u64_t ctz_safe(i64_t n) noexcept {
    return __builtin_ia32_tzcnt_u64(n);
  }
#else
  /// Count trailing zeros (from the least significant bit)
  ///
  /// Result is the number of bits in Int if n is zero.
  template<typename Int>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline i32_t ctz_safe(Int n) noexcept {
    return n ? ctz(n) : bitsizeof<Int>;
  }
#endif

  /// Count leading zeros (from the most significant bit)
  ///
  /// Result is undefined if [n] is zero.
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(u8_t n) noexcept {
    return __builtin_clzs(n) - 8;
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(u16_t n) noexcept {
    return __builtin_clzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(i8_t n) noexcept {
    return __builtin_clzs(n) - 8;
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(char n) noexcept {
    return __builtin_clzs(n) - 8;
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(i16_t n) noexcept {
    return __builtin_clzs(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(u32_t n) noexcept {
    return __builtin_clz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(i32_t n) noexcept {
    return __builtin_clz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(u64_t n) noexcept {
    return __builtin_clzl(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t clz(i64_t n) noexcept {
    return __builtin_ctzl(n);
  }

  /// Count leading zeros (from the most significant bit)
  ///
  /// Result is the number of bits in Int if n is zero.
  template<typename Int>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u32_t clz_safe(Int n) noexcept {
#if J_IS_AMD64
    if constexpr (sizeof(Int) == sizeof(u8_t)) {
      return __builtin_ia32_lzcnt_u16(((u32_t)n << 8) | 0x80U);
    } else if constexpr (sizeof(Int) == sizeof(u16_t)) {
      return __builtin_ia32_lzcnt_u16(n);
    } else if constexpr (sizeof(Int) == sizeof(u32_t)) {
      return __builtin_ia32_lzcnt_u32(n);
    } else {
      static_assert(sizeof(Int) == sizeof(u64_t));
      return __builtin_ia32_lzcnt_u64(n);
    }
#else
    return n ? clz(n) : bitsizeof<Int>;
#endif
  }

  template<typename Int>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u32_t find_last_set(Int n) noexcept {
    return bitsizeof<Int> - 1U - clz(n);
  }

  template<typename Int>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline u32_t ffs(Int n) noexcept {
    if constexpr (sizeof(Int) == sizeof(unsigned long int)) {
      return __builtin_ffsl(n);
    } else {
      static_assert(sizeof(Int) <= sizeof(unsigned int));
      return __builtin_ffs(n);
    }
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline i32_t popcount(u128_t n) noexcept {
    return __builtin_popcountl((u64_t)n) + __builtin_popcountl((u64_t)(n >> 64));
  }

  /// Count 1-bits in [n].
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(u8_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(u16_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(u32_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(u64_t n) noexcept {
    return __builtin_popcountl(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(i8_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(char n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(i16_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(i32_t n) noexcept {
    return __builtin_popcount(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr i32_t popcount(i64_t n) noexcept {
    return __builtin_popcountl(n);
  }

  /// Get a bit mask [bits] wide.
  template<typename T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr T mask(i32_t bits) noexcept {
    return (bits == bitsizeof<T> ? T(0) : (T(1) << bits)) - T(1);
  }

  /// Get a bit mask [bits] wide, starting at bit [offset].
  template<typename T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr T mask(i32_t bits, i32_t offset) noexcept {
    return ((bits == bitsizeof<T> ? T(0) : (T(1) << bits)) - T(1)) << offset;
  }

  template<typename T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline T next_power_of_two(T num) noexcept {
    return T(1) << (sizeof(T) * 8 - clz_safe(num));
  }

  template<typename T>
  J_AV(HIDDEN) inline constexpr int align_bits = ctz(alignof(T));
  template<> J_AV(HIDDEN) inline constexpr  int align_bits<void> = 0;

  /// Get the position of the most significant one-bit.
  ///
  /// Result is undefined if n is zero.
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(i8_t n) noexcept {
    return 8U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(u8_t n) noexcept {
    return 8U - clz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(char n) noexcept {
    return 8U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(i16_t n) noexcept {
    return 16U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(u16_t n) noexcept {
    return 16U - clz(n);
  }
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(i32_t n) noexcept {
    return 32U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(u32_t n) noexcept {
    return 32U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(i64_t n) noexcept {
    return 64U - clz(n);
  }

  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr u8_t bit_width(u64_t n) noexcept {
    return 64U - clz(n);
  }
  template<SignedInteger T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr bool fits_i8(T i) noexcept {
    return (T)(i8_t)(u8_t)i == i;
  }

  template<SignedInteger T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr bool fits_i16(T i) noexcept {
    return (T)(i16_t)(u16_t)i == i;
  }

  template<SignedInteger T>
  J_A(AI,NODISC,FLATTEN,ND,HIDDEN) inline constexpr bool fits_i32(T i) noexcept {
    return (T)(i32_t)(u32_t)i == i;
  }
}
