#pragma once

#include "ansi/ascii_codes.hpp"
#include "colors/ansi.hpp"
#include "strings/string.hpp"
#include "strings/formatting/format_digits.hpp"

namespace j::ansi {
  enum class sgr_command : u8_t {
      reset           = 0U,
      /// Bold or bright.
      bold            = 1U,
      /// Dim color or light font weight.
      faint           = 2U,
      /// Rarely supported, may do something completely else.
      italic          = 3U,
      underline       = 4U,
      slow_blink      = 5U,
      /// Rarely supported.
      rapid_blink     = 6U,
      /// Swap foreground and background colors.
      reverse_video   = 7U,
      /// Hidden characters. Rarely supported.
      conceal         = 8U,
      /// Strike-through.
      crossed_out     = 9U,

      /// First non-bright foreground color
      fg_begin        = 30U,
      fg_black        = fg_begin,
      fg_red,
      fg_green,
      fg_yellow,
      fg_blue,
      fg_magenta,
      fg_cyan,
      fg_white,

      /// 8-bit or 24-bit colors.
      fg_extended_color = 38U,

      /// First bright foreground color
      fg_bright_begin = 90U,
      fg_bright_black = fg_bright_begin,
      fg_bright_red,
      fg_bright_green,
      fg_bright_yellow,
      fg_bright_blue,
      fg_bright_magenta,
      fg_bright_cyan,
      fg_bright_white,

      /// First non-bright background color
      bg_begin        = 40U,
      bg_black        = bg_begin,
      bg_red,
      bg_green,
      bg_yellow,
      bg_blue,
      bg_magenta,
      bg_cyan,
      bg_white,

      /// 8-bit or 24-bit colors.
      bg_extended_color = 48U,

      /// First bright background color
      bg_bright_begin = 100U,
      bg_bright_black = bg_bright_begin,
      bg_bright_red,
      bg_bright_green,
      bg_bright_yellow,
      bg_bright_blue,
      bg_bright_magenta,
      bg_bright_cyan,
      bg_bright_white,
  };

  J_ALWAYS_INLINE_NONNULL char * put_sgr_cmd(char * J_NOT_NULL ptr, sgr_command cmd) noexcept {
    return strings::formatting::format_dec((u8_t)cmd, ptr);
  }

  inline sgr_command fg_color_to_sgr(colors::ansi_color c) noexcept {
    if (c >= colors::ansi_color::bright_start) {
      return static_cast<sgr_command>(
        static_cast<u8_t>(c) - static_cast<u8_t>(colors::ansi_color::bright_start)
        + static_cast<u8_t>(sgr_command::fg_bright_begin));
    } else {
      return static_cast<sgr_command>(
        static_cast<u8_t>(c) + static_cast<u8_t>(sgr_command::fg_begin));
    }
  }

  inline sgr_command bg_color_to_sgr(colors::ansi_color c) noexcept {
    if (c >= colors::ansi_color::bright_start) {
      return static_cast<sgr_command>(
        static_cast<u8_t>(c) - static_cast<u8_t>(colors::ansi_color::bright_start)
        + static_cast<u8_t>(sgr_command::bg_bright_begin));
    } else {
      return static_cast<sgr_command>(
        static_cast<u8_t>(c) + static_cast<u8_t>(sgr_command::bg_begin));
    }
  }
}
