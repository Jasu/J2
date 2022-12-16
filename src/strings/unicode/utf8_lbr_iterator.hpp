#pragma once

#include "strings/unicode/line_breaks.hpp"
#include "strings/unicode/utf8_code_point_iterator.hpp"
#include "strings/string_view.hpp"

namespace j::strings::inline unicode {
  struct utf8_lbr_iterator final {
    utf8_code_point_iterator m_it;
    mutable utf8_code_point_iterator m_end;
    mutable utf8_code_point_iterator m_next;
    mutable line_break_state m_state;
    mutable enum line_break_type m_break_type = line_break_type::forbidden;

    J_ALWAYS_INLINE constexpr utf8_lbr_iterator() noexcept = default;

    J_ALWAYS_INLINE constexpr utf8_lbr_iterator(utf8_code_point_iterator it, utf8_code_point_iterator end) noexcept
      : m_it(it), m_end(end)
    { }

    J_ALWAYS_INLINE constexpr utf8_lbr_iterator(const char * it, const char * end) noexcept
      : m_it{it, end}, m_end{end, end}
    { }

    J_ALWAYS_INLINE constexpr utf8_lbr_iterator(utf8_code_point_iterator it, utf8_code_point_iterator end, line_break_state state) noexcept
      : m_it(it), m_end(end), m_state(state)
    { }

    J_ALWAYS_INLINE constexpr utf8_lbr_iterator(const char * it, const char * end, line_break_state state) noexcept
      : m_it{it, end}, m_end{end, end}, m_state(state)
    { }

    J_INLINE_GETTER const_string_view operator*() const noexcept {
      return const_string_view{m_it.m_it, (i32_t)(get_next().m_it - m_it.m_it)};
    }

    J_INLINE_GETTER enum line_break_type line_break_type() const noexcept {
      return get_next(), m_break_type;
    }

    J_INLINE_GETTER const line_break_state & state() const noexcept{
      return m_state;
    }

    J_ALWAYS_INLINE utf8_lbr_iterator & operator++() noexcept {
      m_it = get_next();
      m_next.reset();
      return *this;
    }

    J_ALWAYS_INLINE utf8_lbr_iterator operator++(int) noexcept {
      const auto result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE operator const utf8_code_point_iterator &() const noexcept
    { return m_it; }

    const utf8_code_point_iterator & get_next() const noexcept {
      if (!m_next) {
        m_next = m_it;
        if (J_UNLIKELY(!m_next)) {
          return m_next;
        }
        u32_t c = *m_next;
        while (J_LIKELY(++m_next < m_end)) {
          m_state.update_state(c);
          c = *m_next;
          m_break_type = m_state.get_break_type(c);
          if (m_break_type != line_break_type::forbidden) {
            m_state.reset_state();
            return m_next;
          }
        }
        m_next = ::j::min(m_next, m_it);
        // UAX #14 requires a mandatory break at end of text.
        m_break_type = line_break_type::mandatory;
      }
      return m_next;
    }

    J_INLINE_GETTER bool operator==(const utf8_lbr_iterator & rhs) const noexcept {
      return m_it == rhs.m_it;
    }
  };
}
