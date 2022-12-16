#pragma once

#include "hzd/integer_traits.hpp"

namespace j {
  template<typename T> inline constexpr T nan_v;
  template<>
  inline constexpr float nan_v<float> = __builtin_nanf("");
  template<>
  inline constexpr float nan_v<double> = __builtin_nan("");

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float sqrt(float v) noexcept
  { return __builtin_sqrtf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double sqrt(double v) noexcept
  { return __builtin_sqrt(v); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float cbrt(float v) noexcept
  { return __builtin_cbrtf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double cbrt(double v) noexcept
  { return __builtin_cbrt(v); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float fmin(float a, float b) noexcept
  { return __builtin_fminf(a, b); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double fmin(double a, double b) noexcept
  { return __builtin_fmin(a, b); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float fmax(float a, float b) noexcept
  { return __builtin_fmaxf(a, b); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double fmax(double a, double b) noexcept
  { return __builtin_fmax(a, b); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float pow(float a, float b) noexcept
  { return __builtin_powf(a, b); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double pow(double a, double b) noexcept
  { return __builtin_pow(a, b); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float pow(float a, int b) noexcept
  { return __builtin_powif(a, b); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double pow(double a, int b) noexcept
  { return __builtin_powi(a, b); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float powi(float a, int b) noexcept
  { return __builtin_powif(a, b); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double powi(double a, int b) noexcept
  { return __builtin_powi(a, b); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN i32_t round_to_i32(float a) noexcept
  { return __builtin_lrintf(a); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN i32_t round_to_i32(double a) noexcept
  { return __builtin_lrint(a); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN i64_t round_to_i64(float a) noexcept
  { return __builtin_llrintf(a); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN i64_t round_to_i64(double a) noexcept
  { return __builtin_llrint(a); }


  J_INLINE_GETTER_NO_DEBUG_FLATTEN float truncate(float v) noexcept
  { return __builtin_truncf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double truncate(double v) noexcept
  { return __builtin_trunc(v); }


  J_INLINE_GETTER_NO_DEBUG_FLATTEN float round(float v) noexcept
  { return __builtin_roundf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double round(double v) noexcept
  { return __builtin_round(v); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float floor(float v) noexcept
  { return __builtin_floorf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double floor(double v) noexcept
  { return __builtin_floor(v); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN float ceil(float v) noexcept
  { return __builtin_ceilf(v); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double ceil(double v) noexcept
  { return __builtin_ceil(v); }


  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_finite(float a) noexcept
  { return __builtin_isfinite(a); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_finite(double a) noexcept
  { return __builtin_isfinite(a); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_normal(float a) noexcept
  { return __builtin_isnormal(a); }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_normal(double a) noexcept
  { return __builtin_isnormal(a); }

  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_normal_or_zero(float a) noexcept
  { return __builtin_isnormal(a) || a == 0.0f; }
  J_INLINE_GETTER_NO_DEBUG_FLATTEN bool is_normal_or_zero(double a) noexcept
  { return __builtin_isnormal(a) || a == 0.0; }


  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T add_overflows(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    return __builtin_add_overflow(a, b, &a);
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T sub_overflows(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    return __builtin_sub_overflow(a, b, &a);
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T mul_overflows(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    return __builtin_mul_overflow(a, b, &a);
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T add_sat(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    T result = 0;
    if (J_UNLIKELY(__builtin_add_overflow(a, b, &result))) {
      if constexpr (is_signed_v<T>) {
        if (a < T(0)) {
          return min_value_v<T>;
        }
      }
      return max_value_v<T>;
    }
    return result;
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T sub_sat(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    T result = 0;
    if (J_UNLIKELY(__builtin_sub_overflow(a, b, &result))) {
      if constexpr (is_signed_v<T>) {
        if (a < T(0)) {
          return min_value_v<T>;
        }
      }
      return max_value_v<T>;
    }
    return result;
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T mul_sat(T a, U b) noexcept {
    static_assert(is_integral_v<T> && is_integral_v<U>);
    T result = 0;
    if (J_UNLIKELY(__builtin_mul_overflow(a, b, &result))) {
      if constexpr (is_signed_v<T>) {
        if (a < T(0) != b < T(0)) {
          return min_value_v<T>;
        }
      }
      return max_value_v<T>;
    }
    return result;
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T add_overflow(T a, U b) noexcept {
    static_assert(is_integral_v<T>);
    return make_unsigned_t<T>(a) + make_unsigned_t<T>(b);
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T sub_overflow(T a, U b) noexcept {
    static_assert(is_integral_v<T>);
    return make_unsigned_t<T>(a) - make_unsigned_t<T>(b);
  }

  template<typename T, typename U>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN T mul_overflow(T a, U b) noexcept {
    static_assert(is_integral_v<T>);
    return make_unsigned_t<T>(a) * make_unsigned_t<T>(b);
  }
}
