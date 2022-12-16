#pragma once

#include "hzd/type_traits.hpp"

namespace j {
  template<typename T> J_NO_DEBUG inline constexpr T max_value_v;
  template<> J_NO_DEBUG inline constexpr i8_t max_value_v<i8_t> = __INT8_MAX__;
  template<> J_NO_DEBUG inline constexpr i16_t max_value_v<i16_t> = __INT16_MAX__;
  template<> J_NO_DEBUG inline constexpr i32_t max_value_v<i32_t> = __INT32_MAX__;
  template<> J_NO_DEBUG inline constexpr i64_t max_value_v<i64_t> = __INT64_MAX__;

  template<> J_NO_DEBUG inline constexpr u8_t max_value_v<u8_t> = __UINT8_MAX__;
  template<> J_NO_DEBUG inline constexpr u16_t max_value_v<u16_t> = __UINT16_MAX__;
  template<> J_NO_DEBUG inline constexpr u32_t max_value_v<u32_t> = __UINT32_MAX__;
  template<> J_NO_DEBUG inline constexpr u64_t max_value_v<u64_t> = __UINT64_MAX__;

  template<typename T> J_NO_DEBUG inline constexpr T min_value_v;
  template<> J_NO_DEBUG inline constexpr i8_t min_value_v<i8_t> = -__INT8_MAX__ - 1;
  template<> J_NO_DEBUG inline constexpr i16_t min_value_v<i16_t> = -__INT16_MAX__ - 1;
  template<> J_NO_DEBUG inline constexpr i32_t min_value_v<i32_t> = -__INT32_MAX__ - 1;
  template<> J_NO_DEBUG inline constexpr i64_t min_value_v<i64_t> = -__INT64_MAX__ - 1LL;

  template<> J_NO_DEBUG inline constexpr u8_t min_value_v<u8_t>   = 0U;
  template<> J_NO_DEBUG inline constexpr u16_t min_value_v<u16_t> = 0U;
  template<> J_NO_DEBUG inline constexpr u32_t min_value_v<u32_t> = 0U;
  template<> J_NO_DEBUG inline constexpr u64_t min_value_v<u64_t> = 0UL;

#ifdef __SIZEOF_INT128__
  template<> J_NO_DEBUG inline constexpr i128_t max_value_v<i128_t> = (i128_t(1) << 126) + ((i128_t(1) << 126) - i128_t(1));
  template<> J_NO_DEBUG inline constexpr i128_t min_value_v<i128_t> = -max_value_v<i128_t> - i128_t(1);
  template<> J_NO_DEBUG inline constexpr u128_t max_value_v<u128_t> = ~u128_t(0);
  template<> J_NO_DEBUG inline constexpr u128_t min_value_v<u128_t> = 0UL;
#endif

  namespace detail {
    template<typename T>
    struct J_TYPE_HIDDEN next_larger;

    template<> struct J_TYPE_HIDDEN next_larger<u8_t> final  { using type J_NO_DEBUG_TYPE = u16_t; };
    template<> struct J_TYPE_HIDDEN next_larger<u16_t> final { using type J_NO_DEBUG_TYPE = u32_t; };
    template<> struct J_TYPE_HIDDEN next_larger<u32_t> final { using type J_NO_DEBUG_TYPE = u64_t; };

    template<> struct J_TYPE_HIDDEN next_larger<i8_t> final  { using type J_NO_DEBUG_TYPE = i16_t; };
    template<> struct J_TYPE_HIDDEN next_larger<i16_t> final { using type J_NO_DEBUG_TYPE = i32_t; };
    template<> struct J_TYPE_HIDDEN next_larger<i32_t> final { using type J_NO_DEBUG_TYPE = i64_t; };

    template<typename T>
    struct J_TYPE_HIDDEN make_unsigned final {
      static_assert(__is_unsigned(T) || __is_floating_point(T));
      using type J_NO_DEBUG_TYPE = T;
    };

    template<> struct J_TYPE_HIDDEN make_unsigned<i8_t> final  { using type J_NO_DEBUG_TYPE = u8_t; };
    template<> struct J_TYPE_HIDDEN make_unsigned<i16_t> final { using type J_NO_DEBUG_TYPE = u16_t; };
    template<> struct J_TYPE_HIDDEN make_unsigned<i32_t> final { using type J_NO_DEBUG_TYPE = u32_t; };
    template<> struct J_TYPE_HIDDEN make_unsigned<i64_t> final { using type J_NO_DEBUG_TYPE = u64_t; };

    template<typename T> struct J_TYPE_HIDDEN make_signed {
      static_assert(__is_signed(T));
      using type J_NO_DEBUG_TYPE = T;
    };

    template<> struct J_TYPE_HIDDEN make_signed<u8_t> final     { using type J_NO_DEBUG_TYPE = i8_t; };
    template<> struct J_TYPE_HIDDEN make_signed<u16_t> final    { using type J_NO_DEBUG_TYPE = i16_t; };
    template<> struct J_TYPE_HIDDEN make_signed<u32_t> final    { using type J_NO_DEBUG_TYPE = i32_t; };
    template<> struct J_TYPE_HIDDEN make_signed<u64_t> final    { using type J_NO_DEBUG_TYPE = i64_t; };

#ifdef __SIZEOF_INT128__
    template<> struct J_TYPE_HIDDEN make_unsigned<i128_t> final { using type J_NO_DEBUG_TYPE = u128_t; };
    template<> struct J_TYPE_HIDDEN make_signed<u128_t> final   { using type J_NO_DEBUG_TYPE = i128_t; };
    template<> struct J_TYPE_HIDDEN next_larger<u64_t> final { using type J_NO_DEBUG_TYPE = u128_t; };
    template<> struct J_TYPE_HIDDEN next_larger<i64_t> final { using type J_NO_DEBUG_TYPE = i128_t; };
#endif
  }

  template<typename T>
  using make_unsigned_t J_NO_DEBUG_TYPE = typename detail::make_unsigned<T>::type;

  template<typename T>
  using make_signed_t J_NO_DEBUG_TYPE = typename detail::make_signed<T>::type;

  template<typename T>
  using next_larger_t J_NO_DEBUG_TYPE = typename detail::next_larger<T>::type;
}
