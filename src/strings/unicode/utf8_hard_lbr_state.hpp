#pragma once

#include "hzd/utility.hpp"
#include "strings/unicode/line_breaks_fwd.hpp"

namespace j::strings::inline unicode {
  /// State machine for detecting hard breaks. This avoids runing the complete
  /// Unicode line-breaking algorithm whe only hard breaks are needed.
  ///
  /// Only the following characters are handled:
  ///
  ///   - U+000A: Line feed / new line (LF)
  ///   - U+000B: Vertical Tab (line tabulation)
  ///   - U+000C: Form feed
  ///   - U+000D: Carriage return (CR)
  ///   - U+2028: Line separator        (UTF-8: E2 80 A8)
  ///   - U+2029: Paragraph separator   (UTF-8: E2 80 A9)
  ///
  /// For CR+LF, a single character of lookahead is required.
  struct utf8_hard_lbr_state final {
    enum class state_t : u8_t {
      /// Not expecting any special character, no line break.
      normal = 0x00U,
      /// The previous character was an unconditional line break.
      bk     = 0x01U,
      /// Expecting a line feed (LF). Any other character will trigger a line break.
      cr     = 0x0DU,
      /// Expecting 0x80 following an 0xE2
      e2     = 0xE2U,
      /// Expecting either A8 or A9 (Line separator or paragraph separator)
      e280   = 0x80U,
    };

    J_BOILERPLATE(utf8_hard_lbr_state, CTOR_CE)

    J_ALWAYS_INLINE explicit constexpr utf8_hard_lbr_state(state_t s) noexcept
      : state(s)
    {
    }

    explicit utf8_hard_lbr_state(const line_break_state & s) noexcept {
      if (s.m_state == lbr_state::bk || s.m_state == lbr_state::lf) {
        state = state_t::bk;
      } else if (s.m_state == lbr_state::cr) {
        state = state_t::cr;
      }
    }

    state_t state = state_t::normal;

    J_INLINE_GETTER bool should_break(u8_t next_byte) const noexcept {
      return state == state_t::bk
          || (state == state_t::cr && next_byte != 0x0AU);
    }

    void update_state(u8_t byte) noexcept {
      switch (byte) {
      case 0x0AU: case 0x0BU: case 0x0CU:
        state = state_t::bk;
        break;
      case 0x0DU:
        state = state_t::cr;
        break;

      case 0xE2U:
        state = state_t::e2;
        break;

      case 0x80U:
        state = state == state_t::e2
          ? state_t::e280
          : state_t::normal;
        break;

      case 0xA8U: case 0xA9U:
        state = state == state_t::e280
          ? state_t::bk
          : state_t::normal;
        break;

      default: state = state_t::normal; break;
      }
    }

    J_ALWAYS_INLINE void reset() noexcept
    { state = state_t::normal; }
  };
}
