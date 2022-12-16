#pragma once

#include "hzd/string.hpp"

namespace j::strings {
  template<typename T>
  sz_t find_char(const T & str, char ch, sz_t start = 0) noexcept {
    const char * const begin = str.data();
    const char * const pos = reinterpret_cast<const char *>(::j::memchr(begin + start, ch, str.size() - start));
    return pos ? pos - begin : T::npos;
  }

  inline bool is_escaped(const char * c, const char * start) noexcept {
    u32_t num_escapes = 0;
    while (--c >= start) {
      if (*c == '\\') {
        ++num_escapes;
      } else {
        break;
      }
    }
    return num_escapes & 1U;
  }

  template<typename T>
  sz_t find_unescaped_char(const T & str, char ch, sz_t start = 0) noexcept {
    const char * const begin = str.data();
    const char * pos = begin + start;
    for (;;) {
      pos = reinterpret_cast<const char *>(memchr(pos, ch, str.size() - (pos - begin)));
      if (!pos) {
        return T::npos;
      } else if (!is_escaped(pos, begin)) {
        return pos - begin;
      } else {
        ++pos;
      }
    }
  }
}
