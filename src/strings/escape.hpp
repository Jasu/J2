#pragma once

#include "strings/string_view.hpp"
#include "strings/string.hpp"

namespace j::strings {
  using escape_table = const_string_view[256];

  [[nodiscard]] inline strings::string escape(strings::const_string_view in, const escape_table & escapes, i32_t max_escape_len) {
    strings::string result{in.size() + max_escape_len};
    char * wr = result.begin();
    char * end = result.end();
    for (char c : in) {
      if (wr + max_escape_len > end) {
        i32_t sz = wr - result.begin();
        string n{result.size() + 128 + max_escape_len};
        ::j::memcpy(n.begin(), result.begin(), sz);
        result = n;
        wr = result.begin() + sz;
        end = result.end();
      }
      if (!escapes[(u8_t)c]) {
        *wr++ = c;
      } else {
        u32_t sz = escapes[(u8_t)c].size();
        ::j::memcpy(wr, escapes[(u8_t)c].begin(), sz);
        wr += escapes[(u8_t)c].size();
      }
    }
    return {result.begin(), wr};
  }

  strings::string escape_double_quoted(strings::const_string_view in) noexcept;
}
