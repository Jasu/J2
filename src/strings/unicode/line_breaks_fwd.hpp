#pragma once

#include "hzd/utility.hpp"

namespace j::strings::inline unicode {
  enum class line_break_type : u8_t {
    /// Uninitialized value
    none,
    forbidden,
    mandatory,
    opportunity,
  };

  enum class lbr_state : u8_t {
    none,
    /// The previous character was a hard break.
    bk,
    /// The previous character was a CR. (\r)
    cr,
    /// The previous character was a LF. (\n)
    lf,
    /// The previous character was a zero-width space, potentially followed by real spaces.
    zw,
    /// The previous character was a zero-width joiner
    zwj,
    /// The previous character was a combining mark.
    cm,
    /// The previous character was a space.
    sp,
    /// The previous character was an alphanumeric character.
    al, // 9
    /// The previous character was a word joiner
    wj,
    /// The previous character was a non-breaking space.
    gl,
    /// The previous character was marked as "break after".
    ba,
    /// The previous character was marked as "break before".
    bb,
    /// The previous character was a hyphen.
    hy,
    /// The previous character was a closing punctuation or paren character (e.g. "}" or ")").
    cl,
    /// The previous character was CL or CP, followed by spaces.
    cl_sp, // 16
    /// The previous character was an opening paren (e.g. ")").
    op,
    /// The previous character was an exclamation mark (e.g. "!" or "?")
    ex,
    /// The previous character was an infix number separator (e.g. "." or ",")
    is, // 19
    /// The previous character was a symbol allowing break after but not before ("/")
    sy,
    /// The previous character was a quotation character.
    qu,
    /// The previous character was a quotation character, followed by spaces.
    qu_sp,
    /// The previous character was marked with break opportunity after, possibly followed by spaces.
    b2,
    /// The previous character was marked as a conditional break.
    cb,
    /// The previous character was a Hebrew letter.
    hl, // 25
    /// The previous character was a Hebrew letter followed by a hyphen or break-after.
    hl_hyba,
    /// The previous character was a number.
    nu,
    /// The previous character was a numeric prefix ($123).
    pr,
    /// The previous character was a numeric postfix (101%).
    po,
    /// The previous character was an ideographic character.
    id, // 30
    /// The previous character was an emoji base.
    eb,
    /// The previous character was an emoji modifier.
    em,
    /// The previous character was a Hangul leading Jamo.
    jl,
    /// The previous character was a Hangul vowel Jamo.
    jv,
    /// The previous character was a Hangul trailing Jamo.
    jt,
    /// The previous character was a precomposed Hangul LV syllable.
    h2,
    /// The previous character was a precomposed Hangul LVT syllable.
    h3,
    /// The previous character was an odd-numbered regional indicator.
    ri_odd,
    /// The previous character was an even-numbered regional indicator.
    ri_even,
    /// The previous character was a closing paren character (e.g. "}" or ")").
    cp_narrow,
    /// The previous character was a closing paren character (e.g. "}" or ")") that was wide.
    cp_wide,
    /// The previous character was a closing paren character and spaces.
    cp_sp, // 16
    /// The previous character was a non-starter.
    ns, // 17
    /// The previous character was an ellipsis.
    in, // 17
    /// The previous character was a b2, followed by space(s).
    b2_sp, // 16
  };

  struct line_break_state final {
    lbr_state m_state = lbr_state::none;
    lbr_state m_eccs_base = lbr_state::none;

    void reset_state() noexcept {
      m_eccs_base = m_state = lbr_state::none;
    }

    [[nodiscard]] line_break_type get_break_type(const u32_t c) const noexcept;

    void update_state(const u32_t c) noexcept;

    J_INLINE_GETTER constexpr bool operator==(const line_break_state & rhs) const noexcept {
      return m_state == rhs.m_state && ((m_state != lbr_state::cm && m_state != lbr_state::zwj) || m_eccs_base == rhs.m_eccs_base);
    }
  };
}
