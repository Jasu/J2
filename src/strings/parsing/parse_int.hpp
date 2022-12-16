#pragma once

#include "hzd/types.hpp"

namespace j::strings {
  class string;
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
  inline namespace parsing {
    template<typename Int> Int parse_decimal_integer(const char * str, u32_t sz);
    template<typename Int> Int parse_decimal_integer_s(const string & string);
    template<typename Int> Int parse_decimal_integer_sv(const const_string_view & string);

    template<>
    [[nodiscard]] u64_t parse_decimal_integer<u64_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] u32_t parse_decimal_integer<u32_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] u16_t parse_decimal_integer<u16_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] u8_t parse_decimal_integer<u8_t>(const char * str, u32_t sz);

    template<>
    [[nodiscard]] i64_t parse_decimal_integer<i64_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] i32_t parse_decimal_integer<i32_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] i16_t parse_decimal_integer<i16_t>(const char * str, u32_t sz);
    template<>
    [[nodiscard]] i8_t parse_decimal_integer<i8_t>(const char * str, u32_t sz);

    template<>
    [[nodiscard]] u64_t parse_decimal_integer_s<u64_t>(const string & string);
    template<>
    [[nodiscard]] u32_t parse_decimal_integer_s<u32_t>(const string & string);
    template<>
    [[nodiscard]] u16_t parse_decimal_integer_s<u16_t>(const string & string);
    template<>
    [[nodiscard]] u8_t parse_decimal_integer_s<u8_t>(const string & string);

    template<>
    [[nodiscard]] i64_t parse_decimal_integer_s<i64_t>(const string & string);
    template<>
    [[nodiscard]] i32_t parse_decimal_integer_s<i32_t>(const string & string);
    template<>
    [[nodiscard]] i16_t parse_decimal_integer_s<i16_t>(const string & string);
    template<>
    [[nodiscard]] i8_t parse_decimal_integer_s<i8_t>(const string & string);

    template<>
    [[nodiscard]] u64_t parse_decimal_integer_sv<u64_t>(const const_string_view & string);
    template<>
    [[nodiscard]] u32_t parse_decimal_integer_sv<u32_t>(const const_string_view & string);
    template<>
    [[nodiscard]] u16_t parse_decimal_integer_sv<u16_t>(const const_string_view & string);
    template<>
    [[nodiscard]] u8_t parse_decimal_integer_sv<u8_t>(const const_string_view & string);

    template<>
    [[nodiscard]] i64_t parse_decimal_integer_sv<i64_t>(const const_string_view & string);
    template<>
    [[nodiscard]] i32_t parse_decimal_integer_sv<i32_t>(const const_string_view & string);
    template<>
    [[nodiscard]] i16_t parse_decimal_integer_sv<i16_t>(const const_string_view & string);
    template<>
    [[nodiscard]] i8_t parse_decimal_integer_sv<i8_t>(const const_string_view & string);
  }
}
