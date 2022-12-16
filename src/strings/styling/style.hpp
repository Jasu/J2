#pragma once

#include "util/flags.hpp"
#include "hzd/types.hpp"
#include "colors/color.hpp"

namespace j::strings::inline styling {
  /// Font-agnostic style flags.
  enum class style_flag : u8_t {
    bold,
    italic,
    strikethrough,
    underline,

    has_bold,
    has_italic,
    has_strikethrough,
    has_underline,
  };

  J_A(HIDDEN) constexpr inline u8_t style_flags_has_mask = 0b11110000U;
}

extern template struct j::util::flags<j::strings::styling::style_flag, u8_t>;

namespace j::strings::inline styling {
  using style_flags_t J_NO_DEBUG_TYPE = util::flags<style_flag, u8_t>;
  J_FLAG_OPERATORS(style_flag, u8_t)

  inline constexpr style_flags_t bold{style_flag::bold, style_flag::has_bold};
  inline constexpr style_flags_t italic{style_flag::italic, style_flag::has_italic};
  inline constexpr style_flags_t strikethrough{style_flag::strikethrough, style_flag::has_strikethrough};
  inline constexpr style_flags_t underline{style_flag::underline, style_flag::has_underline};

  inline constexpr style_flags_t no_bold{style_flag::has_bold};
  inline constexpr style_flags_t no_italic{style_flag::has_italic};
  inline constexpr style_flags_t no_strikethrough{style_flag::has_strikethrough};
  inline constexpr style_flags_t no_underline{style_flag::has_underline};

  struct style final {
    J_BOILERPLATE(style, CTOR_CE, EQ_CE)

    J_ALWAYS_INLINE explicit constexpr style(style_flags_t flags) noexcept
      : flags(flags) { }

    J_ALWAYS_INLINE explicit constexpr style(const colors::color & fg) noexcept
      : foreground(fg)
    { }

    J_ALWAYS_INLINE explicit constexpr style(const colors::color & fg, style_flags_t flags) noexcept
      : flags(flags),
        foreground(fg)
      { }

    J_ALWAYS_INLINE explicit constexpr style(const colors::color & fg, const colors::color & bg) noexcept
      : foreground(fg),
        background(bg)
      { }

    J_ALWAYS_INLINE explicit constexpr style(const colors::color & fg, const colors::color & bg, style_flags_t flags) noexcept
      : flags(flags),
        foreground(fg),
        background(bg)
      { }

    J_A(AI,NODISC,ND,HIDDEN) inline bool empty() const noexcept
    { return !flags && foreground.empty() && background.empty(); }

    J_A(AI,NODISC,ND,HIDDEN) inline constexpr style with_bold() const noexcept
    { return style(foreground, background, flags | bold); }

    style_flags_t flags;
    colors::color foreground;
    colors::color background;
  };

  J_NO_DEBUG inline constexpr style g_empty_style{};

  style override_style(const style & base, const style & overrides) noexcept;
  void override_style_inline(style & base, style overrides) noexcept;
}
