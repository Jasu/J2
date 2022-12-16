#pragma once

#include "strings/unicode/line_breaks.hpp"
#include "strings/unicode/rope/rope_utf8_code_point_iterator.hpp"

namespace j::strings::inline unicode::inline rope {
  struct line_break final {
    rope_utf8_code_point_iterator it;
    line_break_type break_type = line_break_type::none;
  };

  struct rope_utf8_lbr_iterator final {
    line_break m_break;
    line_break_state m_state;

    constexpr rope_utf8_lbr_iterator() noexcept = default;

    constexpr rope_utf8_lbr_iterator(rope_utf8_code_point_iterator it)
      : m_break{it}
    {
      if (m_break.it) {
        m_state.update_state(*m_break.it);
      }
    }

    explicit rope_utf8_lbr_iterator(const_rope_utf8_byte_iterator it)
      : rope_utf8_lbr_iterator(rope_utf8_code_point_iterator(it))
    { }

    J_INLINE_GETTER_NONNULL const line_break * operator->() const noexcept
    { return &m_break; }

    J_INLINE_GETTER const line_break & operator*() const noexcept
    { return m_break; }

    rope_utf8_lbr_iterator & operator++() {
      while (++m_break.it) {
        m_break.break_type = m_state.get_break_type(*m_break.it);
        m_state.update_state(*m_break.it);
        if (m_break.break_type != line_break_type::forbidden) {
          break;
        }
      }
      return *this;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return (bool)m_break.it; }

    J_INLINE_GETTER bool operator!() const noexcept
    { return !m_break.it; }

    J_ALWAYS_INLINE rope_utf8_lbr_iterator operator++(int) {
      const rope_utf8_lbr_iterator result(*this);
      return operator++(), result;
    }

    J_INLINE_GETTER bool operator==(const rope_utf8_lbr_iterator & rhs) const noexcept
    { return m_break.it == rhs.m_break.it; }

    operator rope_utf8_code_point_iterator() const noexcept
    { return m_break.it; }

    operator const_rope_utf8_byte_iterator() const noexcept
    { return m_break.it; }
  };
}
