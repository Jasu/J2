#include "strings/escape.hpp"

namespace j::strings {
  namespace {
    const escape_table double_quoted = {
      [(u8_t)'\0'] = "\\0",
      [(u8_t)'\n'] = "\\n",
      [(u8_t)'\r'] = "\\r",
      [(u8_t)'\t'] = "\\t",
      [(u8_t)'\\'] = "\\",
      [(u8_t)'"'] = "\\\"",
    };
  }

  strings::string escape_double_quoted(strings::const_string_view in) noexcept {
    return escape(in, double_quoted, 2);
  }
}
