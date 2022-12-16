#pragma once

#include "strings/unicode/utf8_hard_lbr_state.hpp"
#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"

namespace j::strings::inline unicode::inline rope {
  template<typename It>
  struct basic_rope_utf8_hard_lbr_iterator final {
    It m_it;
    utf8_hard_lbr_state m_state;

    J_ALWAYS_INLINE constexpr basic_rope_utf8_hard_lbr_iterator() noexcept = default;
    constexpr basic_rope_utf8_hard_lbr_iterator(It it) noexcept
      : m_it(it)
    {
      if (m_it) {
        m_state.update_state(*m_it);
      }
    }
    J_INLINE_GETTER_NONNULL const It * operator->() const noexcept
    { return &m_it; }

    J_INLINE_GETTER const It & operator*() const noexcept
    { return m_it; }

    basic_rope_utf8_hard_lbr_iterator & operator++() {
      while (++m_it) {
        const bool should_break = m_state.should_break(*m_it);
        m_state.update_state(*m_it);
        if (should_break) {
          break;
        }
      }
      return *this;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return m_it.operator bool(); }

    J_INLINE_GETTER bool operator!() const noexcept
    { return m_it.operator!(); }

    J_ALWAYS_INLINE basic_rope_utf8_hard_lbr_iterator operator++(int) {
      const basic_rope_utf8_hard_lbr_iterator result(*this);
      return operator++(), result;
    }

    J_INLINE_GETTER bool operator==(const basic_rope_utf8_hard_lbr_iterator & rhs) const noexcept
    { return m_it == rhs.m_it; }

    J_INLINE_GETTER operator It() noexcept
    { return m_it; }
  };

  using rope_utf8_hard_lbr_iterator = basic_rope_utf8_hard_lbr_iterator<rope_utf8_byte_iterator>;
  using const_rope_utf8_hard_lbr_iterator = basic_rope_utf8_hard_lbr_iterator<const_rope_utf8_byte_iterator>;
}
