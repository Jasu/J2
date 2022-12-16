#pragma once

#include "strings/string_view.hpp"
#include "strings/find.hpp"

namespace j::strings::inline formatting {
  struct placeholder {
    inline explicit placeholder(const_string_view str) noexcept {
      const char * it = str.begin() + 1;
      const char * end = str.end() - 1;
      if (it != end) {
        if (const char * name_end = j::memchr(it, ':', end - it)) {
          formatter_name = const_string_view{it, name_end};
          formatter_options = const_string_view{name_end + 1, end};
        } else {
          formatter_name = const_string_view{it, end};
        }
      }
    }

    const_string_view formatter_name;
    const_string_view formatter_options;
  };
}
