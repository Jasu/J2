#include "strings/styling/style.hpp"

template struct j::util::flags<j::strings::styling::style_flag, u8_t>;

namespace j::strings::inline styling {
  style override_style(const style & base, const style & overrides) noexcept {
    style_flags_t flags;
    if (overrides.flags.has(style_flag::has_bold)) {
      flags |= style_flag::has_bold;
      if (overrides.flags.has(style_flag::bold)) {
        flags |= style_flag::bold;
      }
    } else if (base.flags.has(style_flag::has_bold)) {
      flags |= style_flag::has_bold;
      if (base.flags.has(style_flag::bold)) {
        flags |= style_flag::bold;
      }
    }

    if (overrides.flags.has(style_flag::has_italic)) {
      flags |= style_flag::has_italic;
      if (overrides.flags.has(style_flag::italic)) {
        flags |= style_flag::italic;
      }
    } else if (base.flags.has(style_flag::has_italic)) {
      flags |= style_flag::has_italic;
      if (base.flags.has(style_flag::italic)) {
        flags |= style_flag::italic;
      }
    }


    if (overrides.flags.has(style_flag::has_underline)) {
      flags |= style_flag::has_underline;
      if (overrides.flags.has(style_flag::underline)) {
        flags |= style_flag::underline;
      }
    } else if (base.flags.has(style_flag::has_underline)) {
      flags |= style_flag::has_underline;
      if (base.flags.has(style_flag::underline)) {
        flags |= style_flag::underline;
      }
    }

    if (overrides.flags.has(style_flag::has_strikethrough)) {
      flags |= style_flag::has_strikethrough;
      if (overrides.flags.has(style_flag::strikethrough)) {
        flags |= style_flag::strikethrough;
      }
    } else if (base.flags.has(style_flag::has_strikethrough)) {
      flags |= style_flag::has_strikethrough;
      if (base.flags.has(style_flag::strikethrough)) {
        flags |= style_flag::strikethrough;
      }
    }

    return style(
      overrides.foreground.empty() ? base.foreground : overrides.foreground,
      overrides.background.empty() ? base.background : overrides.background,
      flags);
  }

  void override_style_inline(style & base, style overrides) noexcept {
    u8_t overrides_set = style_flags_has_mask & overrides.flags.value;
    base.flags.value = (base.flags.value & ~(overrides_set >> 4)) | overrides.flags.value;
    if (overrides.foreground) {
      base.foreground = overrides.foreground;
    }
    if (overrides.background) {
      base.background = overrides.background;
    }
  }
}
