#pragma once

#include "strings/formatting/formatter.hpp"
#include "strings/unicode/utf8_string_view.hpp"

namespace j::strings::formatters {
  extern constinit const formatter_known_length<string_view> & g_string_view_formatter;
  extern constinit const formatter_known_length<const_string_view> & g_const_string_view_formatter;
  extern constinit const formatter_known_length<utf8_string_view> & g_utf8_string_view_formatter;
  extern constinit const formatter_known_length<const_utf8_string_view> & g_const_utf8_string_view_formatter;
  extern constinit const formatter_known_length<string> & g_string_formatter;
  extern constinit const formatter_known_length<const char *> & g_const_char_formatter;
}
