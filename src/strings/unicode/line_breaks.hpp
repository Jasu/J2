#pragma once

#include "strings/unicode/line_breaks_fwd.hpp"

#include "strings/unicode/utf8.hpp"
#include <strings/unicode/line_break_property.hpp>
#include <strings/unicode/east_asian_width.hpp>

namespace j::strings::inline unicode {
  /// Return whether s is an acceptable base for an extended combining character sequence.
  J_INLINE_GETTER constexpr bool is_valid_eccs_base(const lbr_state s) noexcept {
    return s != lbr_state::bk && s != lbr_state::cr && s != lbr_state::lf
      && s != lbr_state::sp && s != lbr_state::zw
      && s != lbr_state::qu_sp && s != lbr_state::cl_sp && s != lbr_state::cp_sp
      && s != lbr_state::b2_sp;
  }

  [[nodiscard]] inline line_break_type line_break_state::get_break_type(const u32_t c) const noexcept {
    if (m_state == lbr_state::bk || m_state == lbr_state::lf) {
      return line_break_type::mandatory;
    }
    lbr_t lbr = lbr_category(c);
    auto state = m_state;
    if (state == lbr_state::cr) {
      return lbr == lbr_t::lf ? line_break_type::forbidden : line_break_type::mandatory;
    }
    if (state == lbr_state::none
        || lbr == lbr_t::bk || lbr == lbr_t::cr || lbr == lbr_t::lf
        || lbr == lbr_t::sp || lbr == lbr_t::zw) {
      return line_break_type::forbidden;
    }
    if (state == lbr_state::zw) {
      return line_break_type::opportunity;
    }
    if (state == lbr_state::zwj) {
      return line_break_type::forbidden;
    }

    // LB9 Do not break a combining character sequence; treat it as if it has the line breaking
    //     class of the base character in all of the following rules. Treat ZWJ as if it were CM.
    if (lbr == lbr_t::zwj || lbr == lbr_t::cm) {
      // No need to check for ZWJ here, it was already handled.
      if (state == lbr_state::cm) {
        // Don't break an existing combining character sequence.
        if (m_eccs_base != lbr_state::none) {
          return line_break_type::forbidden;
        } else {
          lbr = lbr_t::al;
        }
      } else {
        // Don't break a starting combining character sequence.
        if (is_valid_eccs_base(state)) {
          return line_break_type::forbidden;
        } else {
          lbr = lbr_t::al;
        }
      }
    }

    if (state == lbr_state::zwj || state == lbr_state::cm) {
      state = J_UNLIKELY(m_eccs_base == lbr_state::none) ? lbr_state::al : m_eccs_base;
    }

    if (state == lbr_state::wj || lbr == lbr_t::wj) {
      return line_break_type::forbidden;
    }
    if (state == lbr_state::gl) {
      return line_break_type::forbidden;
    }
    if (lbr == lbr_t::gl
        && (state != lbr_state::sp && state != lbr_state::b2_sp
            && state != lbr_state::cl_sp && state != lbr_state::cp_sp && state != lbr_state::qu_sp
            && state != lbr_state::ba && state != lbr_state::hy && state != lbr_state::hl_hyba)) {
      return line_break_type::forbidden;
    }

    // LB13 Do not break before ‘]’ or ‘!’ or ‘;’ or ‘/’, even after spaces.
    if (lbr == lbr_t::cl || lbr == lbr_t::cp || lbr == lbr_t::ex || lbr == lbr_t::is || lbr == lbr_t::sy) {
      return line_break_type::forbidden;
    }


    // LB14 Do not break after ‘[’, even after spaces.
    if (state == lbr_state::op) {
      return line_break_type::forbidden;
    }

    // LB15 Don't break after quote, even when followed by spaces if it is followed by an open paren.
    // Lines are not broken around quotes normally, this mainly allows space between an opening quote
    // and an opening paren.
    if (lbr == lbr_t::op && (state == lbr_state::qu || state == lbr_state::qu_sp)) {
      return line_break_type::forbidden;
    }
    // LB16 Do not break between closing punctuation and a nonstarter (lb=NS),
    //      even with intervening spaces.
    if (lbr == lbr_t::ns &&
        (state == lbr_state::cl || state == lbr_state::cp_narrow || state == lbr_state::cp_wide
         || state == lbr_state::cp_sp || state == lbr_state::cl_sp)) {
      return line_break_type::forbidden;
    }
    // LB17 Do not break within ‘——’, even with intervening spaces.
    if ((state == lbr_state::b2 || state == lbr_state::b2_sp) && lbr == lbr_t::b2) {
      return line_break_type::forbidden;
    }
    // LB18 Break after spaces.
    if (state == lbr_state::sp || state == lbr_state::cp_sp || state == lbr_state::cl_sp
        || state == lbr_state::b2_sp || state == lbr_state::qu_sp) {
      return line_break_type::opportunity;
    }
    // LB19 Do not break before or after quotation marks, such as ‘ ” ’.
    if (state == lbr_state::qu || lbr == lbr_t::qu) {
      return line_break_type::forbidden;
    }
    // LB20 Break before and after unresolved CB.
    if (state == lbr_state::cb || lbr == lbr_t::cb) {
      return line_break_type::opportunity;
    }
    // LB21 Do not break before hyphen-minus, other hyphens, fixed-width spaces,
    //      small kana, and other non-starters, or after acute accents.
    if (state == lbr_state::bb || lbr == lbr_t::ba || lbr == lbr_t::hy || lbr == lbr_t::ns) {
      return line_break_type::forbidden;
    }
    // LB21a Don't break after a Hebrew letter + Hyphen.
    if (state == lbr_state::hl_hyba) {
      return line_break_type::forbidden;
    }
    // LB21b Don’t break between Solidus and Hebrew letters.
    if (state == lbr_state::sy && lbr == lbr_t::hl) {
      return line_break_type::forbidden;
    }
    // LB22 Do not break before ellipses.
    if (lbr == lbr_t::in) {
      return line_break_type::forbidden;
    }

    // LB23. Do not break between digits and letters.
    if ((state == lbr_state::al || state == lbr_state::hl) && lbr == lbr_t::nu) {
      return line_break_type::forbidden;
    }
    if ((lbr == lbr_t::al || lbr == lbr_t::hl) && state == lbr_state::nu) {
      return line_break_type::forbidden;
    }

    // LB23a. Do not break between numeric prefixes and ideographs,
    //        or between ideographs and numeric postfixes.
    // N.B. What's the use of this? $SmilingCatEmoji
    if (state == lbr_state::pr && (lbr == lbr_t::id || lbr == lbr_t::em || lbr == lbr_t::eb)) {
      return line_break_type::forbidden;
    }
    if ((state == lbr_state::id || state == lbr_state::em || state == lbr_state::eb) && lbr == lbr_t::po) {
      return line_break_type::forbidden;
    }

    // LB24. Do not break between numeric prefix/postfix and letters,
    //       or between letters and prefix/postfix.
    /// \todo Implement the recommendation on the rule
    if ((state == lbr_state::pr || state == lbr_state::po) && (lbr == lbr_t::al || lbr == lbr_t::hl)) {
      return line_break_type::forbidden;
    }
    if ((lbr == lbr_t::pr || lbr == lbr_t::po) && (state == lbr_state::al || state == lbr_state::hl)) {
      return line_break_type::forbidden;
    }

    // LB25 Do not break between the following pairs of classes relevant to numbers
    //
    // (CL|CP|NU) × (PO|PR)
    // (PO|PR) × (OP|NU)
    // (HY|IS|NU|SY) × NU
    if ((state == lbr_state::cp_narrow || state == lbr_state::cp_wide || state == lbr_state::cl || state == lbr_state::nu) && (lbr == lbr_t::po || lbr == lbr_t::pr)) {
      return line_break_type::forbidden;
    }
    if ((state == lbr_state::po || state == lbr_state::pr) && (lbr == lbr_t::op || lbr == lbr_t::nu)) {
      return line_break_type::forbidden;
    }
    if (lbr == lbr_t::nu && (state == lbr_state::hy || state == lbr_state::is
                             || state == lbr_state::nu || state == lbr_state::sy))
    {
      return line_break_type::forbidden;
    }

    // LB26 & LB27 merged
    //
    // JL × (JL | JV | H2 | H3)
    // (JV | H2) × (JV | JT)
    // (JT | H3) × JT
    //
    // PR × (JL | JV | JT | H2 | H3)
    // (JL | JV | JT | H2 | H3) × (IN | PO)
    //
    // LB22. prohibited all breaks before IN, so it can be removed:
    //
    // JL × (JL | JV | H2 | H3)
    // (JV | H2) × (JV | JT)
    // (JT | H3) × JT
    //
    // PR × (JL | JV | H2 | H3 | JT)
    // (JL | JV | JT | H2 | H3) × PO
    //
    // Becomes
    //
    // (JL | PR) × (JL | JV | H2 | H3)
    // (JV | H2) × (JV | JT)
    // (JT | H3) × JT
    //
    // PR × JT
    // (JL | JV | JT | H2 | H3) × PO
    //
    // Becomes
    //
    // (JL | PR) × (JL | JV | H2 | H3)
    // (JV | H2) × (JV | JT)
    // (JT | H3 | PR) × JT
    //
    // (JL | JV | JT | H2 | H3) × PO
    //
    // Becomes
    //
    // (JL | PR) × (JL | JV | H2 | H3)
    // (JV | H2) × (JV | JT | PO)
    // (JT | H3 | PR) × JT
    //
    // (JL | JT | H3) × PO

    if ((state == lbr_state::jl || state == lbr_state::pr)
        && (lbr == lbr_t::jl || lbr == lbr_t::jv || lbr == lbr_t::h2 || lbr == lbr_t::h3)) {
      return line_break_type::forbidden;
    }

    if ((state == lbr_state::jv || state == lbr_state::h2)
        && (lbr == lbr_t::jv || lbr == lbr_t::jt || lbr == lbr_t::po)) {
      return line_break_type::forbidden;
    }

    if ((state == lbr_state::jt || state == lbr_state::h3 || state == lbr_state::pr)
        && lbr == lbr_t::jt) {
      return line_break_type::forbidden;
    }
    if ((state == lbr_state::jl || state == lbr_state::jt || state == lbr_state::h3)
        && lbr == lbr_t::po) {
      return line_break_type::forbidden;
    }

    // LB28 Do not break between alphabetics (“at”).
    // LB29 Do not break between numeric punctuation and alphabetics (“e.g.”).
    if ((state == lbr_state::al || state == lbr_state::hl || state == lbr_state::is)
        && (lbr == lbr_t::al || lbr == lbr_t::hl)) {
      return line_break_type::forbidden;
    }

    // LB30 Do not break between letters, numbers,
    //      or ordinary symbols and opening or closing parentheses.
    if ((state == lbr_state::al || state == lbr_state::hl || state == lbr_state::nu)
        && lbr == lbr_t::op) {
      auto eaw = get_east_asian_width(c);
      if (eaw != east_asian_width::f && eaw != east_asian_width::w && eaw != east_asian_width::h) {
        return line_break_type::forbidden;
      }
    }
    switch (state) {
    case lbr_state::cp_narrow:
      return (lbr == lbr_t::al || lbr == lbr_t::hl || lbr == lbr_t::nu)
        ? line_break_type::forbidden : line_break_type::opportunity;
    case lbr_state::ri_odd:
      return lbr == lbr_t::ri ? line_break_type::forbidden : line_break_type::opportunity;
    case lbr_state::eb:
      return lbr == lbr_t::em ? line_break_type::forbidden : line_break_type::opportunity;
    default:
      return line_break_type::opportunity;
    }
  }

  inline void line_break_state::update_state(const u32_t c) noexcept {
    const lbr_t lbr = lbr_category(c);
    switch (lbr) {
    case lbr_t::al: m_state = lbr_state::al; break;
    case lbr_t::bk: m_state = lbr_state::bk; break;
    case lbr_t::lf: m_state = lbr_state::lf; break;
    case lbr_t::cr: m_state = lbr_state::cr; break;
    case lbr_t::ns: m_state = lbr_state::ns; break;
    case lbr_t::in: m_state = lbr_state::in; break;

    case lbr_t::nu: m_state = lbr_state::nu; break;

    case lbr_t::zw: m_state = lbr_state::zw; break;
    case lbr_t::gl: m_state = lbr_state::gl; break;

    case lbr_t::bb: m_state = lbr_state::bb; break;

    case lbr_t::hy:
      m_state = m_state == lbr_state::hl ? lbr_state::hl_hyba : lbr_state::hy;
      break;
    case lbr_t::ba:
      m_state = m_state == lbr_state::hl ? lbr_state::hl_hyba : lbr_state::ba;
      break;

    case lbr_t::wj: m_state = lbr_state::wj; break;

    case lbr_t::cl: m_state = lbr_state::cl; break;
    case lbr_t::ex: m_state = lbr_state::ex; break;
    case lbr_t::is: m_state = lbr_state::is; break;
    case lbr_t::sy: m_state = lbr_state::sy; break;
    case lbr_t::cp: {
      auto eaw = get_east_asian_width(c);
      if (eaw != east_asian_width::f && eaw != east_asian_width::w && eaw != east_asian_width::h) {
        m_state = lbr_state::cp_narrow;
      } else {
        m_state = lbr_state::cp_wide;
      }
      break;
    }
    case lbr_t::op:
      m_state = lbr_state::op;
      break;
    case lbr_t::qu: m_state = lbr_state::qu; break;
    case lbr_t::hl: m_state = lbr_state::hl; break;

    case lbr_t::b2: m_state = lbr_state::b2; break;
    case lbr_t::cb: m_state = lbr_state::cb; break;

    case lbr_t::pr: m_state = lbr_state::pr; break;
    case lbr_t::po: m_state = lbr_state::po; break;

    case lbr_t::id: m_state = lbr_state::id; break;
    case lbr_t::eb: m_state = lbr_state::eb; break;
    case lbr_t::em: m_state = lbr_state::em; break;

    case lbr_t::jl: m_state = lbr_state::jl; break;
    case lbr_t::jv: m_state = lbr_state::jv; break;
    case lbr_t::jt: m_state = lbr_state::jt; break;
    case lbr_t::h2: m_state = lbr_state::h2; break;
    case lbr_t::h3: m_state = lbr_state::h3; break;

    case lbr_t::ri:
      m_state = m_state == lbr_state::ri_odd ? lbr_state::ri_even : lbr_state::ri_odd;
      break;

    case lbr_t::sp: {
      auto state = J_UNLIKELY(m_state == lbr_state::cm || m_state == lbr_state::zwj)
        ? (J_UNLIKELY(m_eccs_base == lbr_state::none) ? lbr_state::al : m_eccs_base)
        : m_state;
      if (state != lbr_state::zw && state != lbr_state::op) {
        if (state == lbr_state::qu || state == lbr_state::qu_sp) {
          m_state = lbr_state::qu_sp;
        } else if (state == lbr_state::cl || state == lbr_state::cl_sp) {
          m_state = lbr_state::cl_sp;
        } else if (state == lbr_state::cp_narrow || state == lbr_state::cp_wide || state == lbr_state::cp_sp) {
          m_state = lbr_state::cp_sp;
        } else if (state == lbr_state::b2 || state == lbr_state::b2_sp) {
          m_state = lbr_state::b2_sp;
        } else {
          m_state = lbr_state::sp;
        }
      }
      break;
    }

    case lbr_t::zwj:
    case lbr_t::cm:
      if (m_state != lbr_state::zwj && m_state != lbr_state::cm) {
        if (J_UNLIKELY(!is_valid_eccs_base(m_state))) {
          // Starting a defective combining sequence with no valid base.
          // Each following ZWJ or CM should be treated as AL (after rule LB10, before it
          // ZWJ-specific behavior applies.
          m_eccs_base = lbr_state::none;
        } else {
          m_eccs_base = m_state;
        }
      }
      m_state = lbr == lbr_t::cm ? lbr_state::cm : lbr_state::zwj;
      break;

    default:
      m_state = lbr_state::none;
      break;
    }
  }
}
