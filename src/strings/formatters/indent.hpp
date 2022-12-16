#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::strings::formatters {
  extern const formatter_known_length<i8_t> &  g_indent_formatter_i8_t;
  extern const formatter_known_length<i16_t> & g_indent_formatter_i16_t;
  extern const formatter_known_length<i32_t> & g_indent_formatter_i32_t;
  extern const formatter_known_length<i64_t> & g_indent_formatter_i64_t;
  extern const formatter_known_length<u8_t> &  g_indent_formatter_u8_t;
  extern const formatter_known_length<u16_t> & g_indent_formatter_u16_t;
  extern const formatter_known_length<u32_t> & g_indent_formatter_u32_t;
  extern const formatter_known_length<u64_t> & g_indent_formatter_u64_t;
}
