#pragma once

#include "strings/string.hpp"

namespace j::strings {
  inline string whitelist(const_string_view str, const_string_view list, char replacement) {
    string result;
    if (!replacement) {
      u32_t size = 0;
      for (auto c : str) {
        for (auto w : list) {
          if (w == c) {
            ++size;
            break;
          }
        }
      }
      result = string(size);
    } else {
      result = string(str.size());
    }

    char * ptr = result.data();
    for (auto c : str) {
      bool did_find = false;
      for (auto w : list) {
        if (w == c) {
          *ptr++ = c;
          did_find = true;
          break;
        }
      }
      if (!did_find && replacement) {
        *ptr++ = replacement;
      }
    }

    return result;
  }
}
