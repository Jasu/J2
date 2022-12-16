#include "strings/styling/styled_string.hpp"

namespace j::strings::inline styling {
  void styled_string::clear() noexcept {
    string.clear();
    style.clear();
  }
  styled_string_iterator styled_string::begin() const noexcept {
    auto first = style.begin();
    auto next = first;
    if (next != style.end()) {
      // If the first style points past the first character, the first part has
      // the default style and the begin() iterator actually describes the second
      // part. Set the current iterator to end() and next to begin() in this case.
      if (first->position) {
        first = style.end();
      } else {
        ++next;
      }
    }
    return styled_string_iterator{0, first, next, this};
  }

  styled_string make_styled_string(const style & s, const strings::const_string_view & str) noexcept {
    return {str, string_style{(string_style_segment[]){{0U, s}}}};
  }
}
