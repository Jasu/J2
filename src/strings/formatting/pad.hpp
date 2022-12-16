#pragma once

#include "strings/formatting/alignment.hpp"
#include "strings/string_view.hpp"

namespace j::streams {
  class sink;
}

namespace j::strings::inline formatting {
  struct pad_settings {
    u8_t pad_to = 0U;
    u8_t pad_left = 0U;
    u8_t pad_right = 0U;
    bool zero:1 = false;
    enum alignment alignment:2 = alignment::left;
    constexpr pad_settings() noexcept = default;
    constexpr pad_settings(u8_t pad_to, u8_t pad_left, u8_t pad_right, enum alignment alignment = alignment::left) noexcept
      : pad_to(pad_to),
        pad_left(pad_left),
        pad_right(pad_right),
        alignment(alignment)
    { }
    constexpr pad_settings(u8_t pad_to, enum alignment alignment = alignment::left) noexcept
      : pad_to(pad_to),
        alignment(alignment)
    { }

    constexpr pad_settings with_pad_to(u8_t pad_to) const noexcept {
      return pad_settings(pad_to, pad_left, pad_right, alignment);
    }
  };

  constexpr inline pad_settings left(u8_t pad_to, u8_t pad_left = 0U, u8_t pad_right = 0U) noexcept {
    return pad_settings{pad_to, pad_left, pad_right, alignment::left};
  }

  constexpr inline pad_settings right(u8_t pad_to, u8_t pad_left = 0U, u8_t pad_right = 0U) noexcept {
    return pad_settings{pad_to, pad_left, pad_right, alignment::right};
  }

  constexpr inline pad_settings center(u8_t pad_to, u8_t pad_left = 0U, u8_t pad_right = 0U) noexcept {
    return pad_settings{pad_to, pad_left, pad_right, alignment::center};
  }

  constexpr inline pad_settings around(u8_t pad) noexcept {
    return pad_settings{0U, pad, pad, alignment::left};
  }

  constexpr inline pad_settings around(u8_t pad_left, u8_t pad_right) noexcept {
    return pad_settings{0U, pad_left, pad_right, alignment::left};
  }

  [[nodiscard]] const_string_view get_spaces(i32_t count) noexcept;
  [[nodiscard]] const_string_view get_zeroes(i32_t count) noexcept;

  void write_spaces(streams::sink & target, i32_t count);
  void write_zeroes(streams::sink & target, u32_t count);

  void write_left_padding(streams::sink & target, const pad_settings & pad_settings, u32_t length);
  void write_right_padding(streams::sink & target, const pad_settings & pad_settings, u32_t length);
}
