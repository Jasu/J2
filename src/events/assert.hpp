#pragma once

#include "tags/tag.hpp"

namespace j::events {
  [[noreturn]] void fail_function(const char *function_name, int error);

  extern const tags::tag_definition<int> uv_error;
}

#define J_UV_CALL_CHECKED(FN, ...)                    \
  do {                                                \
    if (auto fn_res = FN(__VA_ARGS__)) { [[unlikely]] \
      fail_function(#FN, fn_res);                     \
    }                                                 \
  } while(false)
