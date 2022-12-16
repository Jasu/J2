#pragma once

#include "hzd/types.hpp"

namespace j::cli {
  struct argument_definition final {
    u32_t index = 0U;
    bool is_option = false;
    bool is_multiple = false;
    bool is_required = false;
    bool has_value = false;
    const char * short_description = nullptr;
    const char * long_description = nullptr;
  };
}
