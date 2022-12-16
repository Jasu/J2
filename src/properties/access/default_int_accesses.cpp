#include "properties/access/int_access_registration.hpp"
#include "properties/assert.hpp"
#include "strings/string.hpp"
#include "strings/formatting/number_format.hpp"
#include "strings/parsing/parse_int.hpp"
#include "hzd/integer_traits.hpp"
#include "hzd/math.hpp"

namespace j::properties::access {
  namespace {
    template<typename T>
    J_NO_INLINE void check_float(T f) {
      if (J_UNLIKELY(!is_normal_or_zero(f))) {
        throw_invalid_conversion("Subnormal, inf, or NaN floating-point numbers cannot be converted to integers.");
      }
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_float_to_int_checked(From from) {
      check_float(from);
      To result = from;
      if (J_UNLIKELY((From)result != from)) {
        throw_invalid_conversion("Floating point number too large.");
      }
      return result;
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_int_to_int_unchecked(From from) noexcept {
      return from;
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_sint_to_sint_checked(From from) {
      if (J_UNLIKELY(from < min_value_v<To>)) {
        throw_invalid_conversion("Source integer is too small.");
      } else if (J_UNLIKELY(from > max_value_v<To>)) {
        throw_invalid_conversion("Source integer is too large.");
      }
      return from;
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_u_to_sint_checked(From from) {
      if (J_UNLIKELY(from > max_value_v<To>)) {
        throw_invalid_conversion("Source integer is too large.");
      }
      return from;
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_sint_to_u_checked(From from) {
      if (J_UNLIKELY(from < 0)) {
        throw_invalid_conversion("Source integer is negative.");
      }
      if constexpr (sizeof(From) >= sizeof(To)) {
        if (J_UNLIKELY(static_cast<To>(from) > max_value_v<To>)) {
          throw_invalid_conversion("Source integer is too large.");
        }
      }
      return from;
    }

    template<typename To, typename From>
    J_INLINE_GETTER To convert_u_to_u_checked(From from) {
      To result(from);
      if (J_UNLIKELY(result != from)) {
        throw_invalid_conversion("Source integer is too large.");
      }
      return result;
    }

    int_access_registration<i8_t> i8_t_access_registration(
      "i8_t",
      convertible_from = &convert_sint_to_sint_checked<i8_t, i16_t>,
      convertible_from = &convert_sint_to_sint_checked<i8_t, i32_t>,
      convertible_from = &convert_sint_to_sint_checked<i8_t, i64_t>,
      convertible_from = &convert_u_to_sint_checked<i8_t, u8_t>,
      convertible_from = &convert_u_to_sint_checked<i8_t, u16_t>,
      convertible_from = &convert_u_to_sint_checked<i8_t, u32_t>,
      convertible_from = &convert_u_to_sint_checked<i8_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<i8_t, float>,
      convertible_from = &convert_float_to_int_checked<i8_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<i8_t>,
      convertible_from = &strings::parse_decimal_integer_sv<i8_t>,

      convertible_to = (strings::string (*)(i8_t))strings::number_format
      );
    int_access_registration<i16_t> i16_t_access_registration(
      "i16_t",
      convertible_from = &convert_int_to_int_unchecked<i16_t, i8_t>,


      convertible_from = &convert_sint_to_sint_checked<i16_t, i32_t>,
      convertible_from = &convert_sint_to_sint_checked<i16_t, i64_t>,
      convertible_from = &convert_int_to_int_unchecked<i16_t, u8_t>,
      convertible_from = &convert_u_to_sint_checked<i16_t, u16_t>,
      convertible_from = &convert_u_to_sint_checked<i16_t, u32_t>,
      convertible_from = &convert_u_to_sint_checked<i16_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<i16_t, float>,
      convertible_from = &convert_float_to_int_checked<i16_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<i16_t>,
      convertible_from = &strings::parse_decimal_integer_sv<i16_t>,

      convertible_to = (strings::string (*)(i16_t))strings::number_format
      );
    int_access_registration<i32_t> i32_t_access_registration(
      "i32_t",
      convertible_from = &convert_int_to_int_unchecked<i32_t, i8_t>,
      convertible_from = &convert_int_to_int_unchecked<i32_t, i16_t>,
      convertible_from = &convert_sint_to_sint_checked<i32_t, i64_t>,
      convertible_from = &convert_int_to_int_unchecked<i32_t, u8_t>,
      convertible_from = &convert_int_to_int_unchecked<i32_t, u16_t>,
      convertible_from = &convert_u_to_sint_checked<i32_t, u32_t>,
      convertible_from = &convert_u_to_sint_checked<i32_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<i32_t, float>,
      convertible_from = &convert_float_to_int_checked<i32_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<i32_t>,
      convertible_from = &strings::parse_decimal_integer_sv<i32_t>,

      convertible_to = (strings::string (*)(i32_t))strings::number_format
      );
    int_access_registration<i64_t> i64_t_access_registration(
      "i64_t",
      convertible_from = &convert_int_to_int_unchecked<i64_t, i8_t>,
      convertible_from = &convert_int_to_int_unchecked<i64_t, i16_t>,
      convertible_from = &convert_int_to_int_unchecked<i64_t, i32_t>,
      convertible_from = &convert_int_to_int_unchecked<i64_t, u8_t>,
      convertible_from = &convert_int_to_int_unchecked<i64_t, u16_t>,
      convertible_from = &convert_int_to_int_unchecked<i64_t, u32_t>,
      convertible_from = &convert_u_to_sint_checked<i64_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<i64_t, float>,
      convertible_from = &convert_float_to_int_checked<i64_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<i64_t>,
      convertible_from = &strings::parse_decimal_integer_sv<i64_t>,

      convertible_to = (strings::string (*)(i64_t))strings::number_format
      );
    int_access_registration<u8_t> u8_t_access_registration(
      "u8_t",
      convertible_from = &convert_sint_to_u_checked<u8_t, i8_t>,
      convertible_from = &convert_sint_to_u_checked<u8_t, i16_t>,
      convertible_from = &convert_sint_to_u_checked<u8_t, i32_t>,
      convertible_from = &convert_sint_to_u_checked<u8_t, i64_t>,
      convertible_from = &convert_u_to_u_checked<u8_t, u16_t>,
      convertible_from = &convert_u_to_u_checked<u8_t, u32_t>,
      convertible_from = &convert_u_to_u_checked<u8_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<u8_t, float>,
      convertible_from = &convert_float_to_int_checked<u8_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<u8_t>,
      convertible_from = &strings::parse_decimal_integer_sv<u8_t>,

      convertible_to = (strings::string (*)(u8_t))strings::number_format
      );
    int_access_registration<u16_t> u16_t_access_registration(
      "u16_t",
      convertible_from = &convert_sint_to_u_checked<u16_t, i8_t>,
      convertible_from = &convert_sint_to_u_checked<u16_t, i16_t>,
      convertible_from = &convert_sint_to_u_checked<u16_t, i32_t>,
      convertible_from = &convert_sint_to_u_checked<u16_t, i64_t>,
      convertible_from = &convert_int_to_int_unchecked<u16_t, u8_t>,
      convertible_from = &convert_u_to_u_checked<u16_t, u32_t>,
      convertible_from = &convert_u_to_u_checked<u16_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<u16_t, float>,
      convertible_from = &convert_float_to_int_checked<u16_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<u16_t>,
      convertible_from = &strings::parse_decimal_integer_sv<u16_t>,

      convertible_to = (strings::string (*)(u16_t))strings::number_format
      );
    int_access_registration<u32_t> u32_t_access_registration(
      "u32_t",
      convertible_from = &convert_sint_to_u_checked<u32_t, i8_t>,
      convertible_from = &convert_sint_to_u_checked<u32_t, i16_t>,
      convertible_from = &convert_sint_to_u_checked<u32_t, i32_t>,
      convertible_from = &convert_sint_to_u_checked<u32_t, i64_t>,
      convertible_from = &convert_int_to_int_unchecked<u32_t, u8_t>,
      convertible_from = &convert_int_to_int_unchecked<u32_t, u16_t>,
      convertible_from = &convert_u_to_u_checked<u32_t, u64_t>,

      convertible_from = &convert_float_to_int_checked<u32_t, float>,
      convertible_from = &convert_float_to_int_checked<u32_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<u32_t>,
      convertible_from = &strings::parse_decimal_integer_sv<u32_t>,

      convertible_to = (strings::string (*)(u32_t))strings::number_format
      );
    int_access_registration<u64_t> u64_t_access_registration(
      "u64_t",
      convertible_from = &convert_sint_to_u_checked<u64_t, i8_t>,
      convertible_from = &convert_sint_to_u_checked<u64_t, i16_t>,
      convertible_from = &convert_sint_to_u_checked<u64_t, i32_t>,
      convertible_from = &convert_sint_to_u_checked<u64_t, i64_t>,
      convertible_from = &convert_int_to_int_unchecked<u64_t, u8_t>,
      convertible_from = &convert_int_to_int_unchecked<u64_t, u16_t>,
      convertible_from = &convert_int_to_int_unchecked<u64_t, u32_t>,

      convertible_from = &convert_float_to_int_checked<u64_t, float>,
      convertible_from = &convert_float_to_int_checked<u64_t, double>,

      convertible_from = &strings::parse_decimal_integer_s<u64_t>,
      convertible_from = &strings::parse_decimal_integer_sv<u64_t>,

      convertible_to = (strings::string (*)(u64_t))strings::number_format
      );
  }

  template<> const wrappers::wrapper_definition * wrapper_definition_v<u8_t> = &int_access_registration<u8_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<u16_t> = &int_access_registration<u16_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<u32_t> = &int_access_registration<u32_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<u64_t> = &int_access_registration<u64_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<i8_t> = &int_access_registration<i8_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<i16_t> = &int_access_registration<i16_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<i32_t> = &int_access_registration<i32_t>::wrapper;
  template<> const wrappers::wrapper_definition * wrapper_definition_v<i64_t> = &int_access_registration<i64_t>::wrapper;

  template<> const typed_access_definition * typed_access_definition_v<u8_t> = &int_access_registration<u8_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<u16_t> = &int_access_registration<u16_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<u32_t> = &int_access_registration<u32_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<u64_t> = &int_access_registration<u64_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<i8_t> = &int_access_registration<i8_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<i16_t> = &int_access_registration<i16_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<i32_t> = &int_access_registration<i32_t>::definition;
  template<> const typed_access_definition * typed_access_definition_v<i64_t> = &int_access_registration<i64_t>::definition;
}
