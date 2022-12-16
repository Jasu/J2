#pragma once

#include "strings/unicode/utf8.hpp"
#include <strings/unicode/grapheme_properties.hpp>
#include <strings/unicode/emoji_data.hpp>

namespace j::strings::inline unicode {
  /// GB3: Carriage return, to not break between CRLF
  inline constexpr u8_t gcs_cr                 = 1U;
  inline constexpr u8_t gcs_lf                 = 2U;
  inline constexpr u8_t gcs_control            = 3U;
  inline constexpr u8_t gcs_prepend            = 4U;
  /// GB11: Extended pictographic or Extend following it
  inline constexpr u8_t gcs_extend             = 5U;
  /// GB11: Zero-width joiner following (Extended pictographic Extend*)
  inline constexpr u8_t gcs_zwj_after_extend   = 6U;
  /// GB6: Hangul syllable L
  inline constexpr u8_t gcs_hangul_l           = 7U;
  /// GB6: Hangul syllable LV or V
  inline constexpr u8_t gcs_hangul_lv_or_v     = 8U;
  /// GB6: Hangul syllable LVT or T
  inline constexpr u8_t gcs_hangul_lvt_or_t    = 9U;
  inline constexpr u8_t gcs_regional_indicator = 10U;

  J_INLINE_GETTER constexpr bool is_hangul_syllable(const u32_t c) noexcept {
    return c >= 0x1100 && c <= 0xD7AF && (c <= 0x1200 || c >= 0xAC00);
  }

  struct grapheme_cluster_state final {
    u8_t char_before = 0U;

    J_ALWAYS_INLINE void reset_state() noexcept {
      char_before = 0U;
    }

    [[nodiscard]] bool can_break(const u32_t c) const noexcept {
      switch (char_before) {
      case gcs_cr:
        return c != '\n';
      case gcs_lf:
        return true;
      case gcs_control:
        return true;
      default:
        break;
      }

      if (c == '\r' || c == '\n' || is_control(c)) {
        return true;
      }

      switch (char_before) {
      case gcs_hangul_l:
        if (is_hangul_syllable(c) && !is_hangul_syllable_t(c)) {
          return false;
        }
        break;
      case gcs_hangul_lv_or_v:
        if (is_hangul_syllable_v(c) || is_hangul_syllable_t(c)) {
          return false;
        }
        break;
      case gcs_hangul_lvt_or_t:
        if (is_hangul_syllable_t(c)) {
          return false;
        }
        break;
      case gcs_prepend:
        return false;
      default:
        break;
      }
      if (is_extend(c) || is_spacing_mark(c) || c == code_point_zwj) {
        return false;
      }
      if (char_before == gcs_zwj_after_extend) {
        return !is_extended_pictographic(c);
      }
      return char_before != gcs_regional_indicator || !is_regional_indicator(c);
    }

    void update_state(const u32_t c) noexcept {
      if (c == '\r') {
        char_before = gcs_cr;
      } else if (c == '\n') {
        char_before = gcs_lf;
      } else if (is_control(c)) {
        char_before = gcs_control;
      } else if (is_hangul_syllable(c)) {
        if (is_hangul_syllable_l(c)) {
          char_before = gcs_hangul_l;
        } else if (is_hangul_syllable_lv(c) || is_hangul_syllable_v(c)) {
          char_before = gcs_hangul_lv_or_v;
        } else {
          char_before = gcs_hangul_lvt_or_t;
        }
      } else if (is_prepend(c)) {
        char_before = gcs_prepend;
      } else if (is_extended_pictographic(c)) {
        char_before = gcs_extend;
      } else if (c == code_point_zwj) {
        char_before = char_before == gcs_extend ? gcs_zwj_after_extend : 0U;
      } else if (is_extend(c)) {
        if (char_before != gcs_extend) {
          char_before = 0U;
        }
      } else if (is_regional_indicator(c)) {
        char_before = char_before == gcs_regional_indicator ? 0U : gcs_regional_indicator;
      } else {
        char_before = 0U;
      }
    }
  };
}
