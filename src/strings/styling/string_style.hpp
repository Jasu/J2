#pragma once

#include "containers/vector.hpp"
#include "strings/styling/style.hpp"
namespace j::strings::inline styling {
  struct string_style_segment final {
    using zero_initializable_tag_t = void;

    J_ALWAYS_INLINE string_style_segment() noexcept = default;

    J_ALWAYS_INLINE constexpr string_style_segment(u32_t pos, style sty) noexcept
      : position(pos), style(sty)
    { }
    u32_t position;
    style style;
  };
}

J_DECLARE_EXTERN_VECTOR(j::strings::string_style_segment);

namespace j::strings {
  inline namespace styling {
    using string_style = vector<string_style_segment>;
  }
}
