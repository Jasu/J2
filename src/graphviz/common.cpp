#include "common.hpp"
#include "strings/formatting/format_digits.hpp"

namespace j::graphviz {
  color::color(colors::rgb8 color) noexcept {
    value[0] = '#';
    strings::format_hex2(color.r, value + 1);
    strings::format_hex2(color.g, value + 3);
    strings::format_hex2(color.b, value + 5);
  }
}
