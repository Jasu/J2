#pragma once

#include "colors/ansi.hpp"
#include "colors/rgb.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::colors {
  struct color final {
    J_ALWAYS_INLINE_NO_DEBUG constexpr color() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr color(const rgb8 & rgb_color) noexcept
      : rgb(rgb_color),
        has_rgb(true)
    {
    }

    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr color(ansi_color ansi_color) noexcept
      : has_ansi(true),
        ansi(ansi_color)
    {
    }

    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr color(rgb8 rgb_color, ansi_color ansi_color) noexcept
      : rgb(rgb_color),
        has_rgb(true),
        has_ansi(true),
        ansi(ansi_color)
    {
    }

    J_INLINE_GETTER_NO_DEBUG constexpr bool empty() const noexcept { return !has_rgb && !has_ansi; }
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return has_rgb || has_ansi; }

    J_INLINE_GETTER constexpr bool operator==(const color & rhs) const noexcept = default;

    colors::rgb8 rgb{0U, 0U, 0U};
    bool has_rgb:1 = false;
    bool has_ansi:1 = false;
    ansi_color ansi:6 = ansi_color::black;
  };
  static_assert(sizeof(color) == 4);
}
