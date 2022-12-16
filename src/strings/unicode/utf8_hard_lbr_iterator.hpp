#pragma once

#include "strings/unicode/utf8_hard_lbr_state.hpp"

namespace j::strings::inline unicode {
  template<typename Ch>
  struct basic_utf8_hard_lbr_iterator final {
    Ch *m_it, *m_end;
    utf8_hard_lbr_state m_state;

    J_ALWAYS_INLINE constexpr basic_utf8_hard_lbr_iterator() noexcept = default;
    constexpr basic_utf8_hard_lbr_iterator(Ch * it, Ch * end) noexcept
      : m_it(it), m_end(end)
    {
      if (m_it && m_it != m_end) {
        m_state.update_state(*m_it);
      }
    }

    J_A(AI,NODISC,RNN) Ch * operator*() const noexcept
    { return m_it; }

    basic_utf8_hard_lbr_iterator & operator++() {
      while (++m_it != m_end) {
        const bool should_break = m_state.should_break(*m_it);
        m_state.update_state(*m_it);
        if (should_break) {
          break;
        }
      }
      return *this;
    }

    J_ALWAYS_INLINE basic_utf8_hard_lbr_iterator operator++(int) {
      const basic_utf8_hard_lbr_iterator result(*this);
      return operator++(), result;
    }

    [[nodiscard]] inline bool operator==(const basic_utf8_hard_lbr_iterator & rhs) const noexcept
    { return m_it == rhs.m_it; }

    J_A(AI,RNN,NODISC) operator Ch*() const noexcept {
      return m_it;
    }
  };

  using utf8_hard_lbr_iterator = basic_utf8_hard_lbr_iterator<char>;
  using const_utf8_hard_lbr_iterator = basic_utf8_hard_lbr_iterator<const char>;
}
