#pragma once

#include "strings/styling/string_style.hpp"
#include "strings/string.hpp"

namespace j::strings::inline styling {
  struct styled_string;

  struct styled_string_iterator final {
    const_string_view string() const noexcept;
    const style & style() const noexcept;
    J_ALWAYS_INLINE const styled_string_iterator & operator*() const noexcept {
      return *this;
    }
    J_ALWAYS_INLINE const styled_string_iterator * operator->() const noexcept {
      return this;
    }
    styled_string_iterator & operator++() noexcept;
    styled_string_iterator operator++(int) noexcept {
      auto res = *this;
      operator++();
      return res;
    }


    J_INLINE_GETTER bool operator==(const styled_string_iterator & rhs) const noexcept
    { return m_start == rhs.m_start; }

    i32_t m_start;
    const j::strings::string_style_segment* m_it;
    const j::strings::string_style_segment* m_it_next;
    const styled_string * m_string;
  };

  struct styled_string final {
    using nicely_copyable_tag_t J_NO_DEBUG_TYPE = void;
    using zero_initializable_tag_t J_NO_DEBUG_TYPE = void;

    styled_string_iterator begin() const noexcept;
    styled_string_iterator end() const noexcept;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return (bool)string;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !(bool)string;
    }

    void clear() noexcept;

    strings::string string;
    string_style style = {};
  };

  styled_string make_styled_string(const style & s, const strings::const_string_view & str) noexcept;

  inline const_string_view styled_string_iterator::string() const noexcept {
    return const_string_view(
      m_string->string.data() + m_start,
      (m_it_next == m_string->style.end() ? m_string->string.size() : m_it_next->position)
      - m_start);
  }

  inline const style & styled_string_iterator::style() const noexcept {
    return m_it == m_string->style.end() ? g_empty_style : m_it->style;
  }

  inline styled_string_iterator & styled_string_iterator::operator++() noexcept {
    m_it = m_it_next;
    if (m_it_next != m_string->style.end()) {
      m_start = m_it_next->position;
      ++m_it_next;
    } else {
      m_start = m_string->string.size();
    }
    return *this;
  }

  inline styled_string_iterator styled_string::end() const noexcept {
    return styled_string_iterator{string.size(), nullptr, nullptr, this};
  }
}
