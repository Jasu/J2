#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::strings::formatters {
  template<typename T> extern const formatter_known_length<T> & integer_formatter_v;
  template<> extern const formatter_known_length<u64_t> & integer_formatter_v<u64_t>;
  template<> extern const formatter_known_length<u32_t> & integer_formatter_v<u32_t>;
  template<> extern const formatter_known_length<u16_t> & integer_formatter_v<u16_t>;
  template<> extern const formatter_known_length<u8_t> & integer_formatter_v<u8_t>;
  template<> extern const formatter_known_length<i64_t> & integer_formatter_v<i64_t>;
  template<> extern const formatter_known_length<i32_t> & integer_formatter_v<i32_t>;
  template<> extern const formatter_known_length<i16_t> & integer_formatter_v<i16_t>;
  template<> extern const formatter_known_length<i8_t> & integer_formatter_v<i8_t>;
}
