#pragma once

#include "strings/string_view.hpp"

namespace j::strings {
  inline namespace styling {
    struct style;
  }
  inline namespace formatting {
    class format_value;
    void parse_push_style(const_string_view token, style & style, const format_value * params, u32_t & current_param) noexcept;
  }
}
