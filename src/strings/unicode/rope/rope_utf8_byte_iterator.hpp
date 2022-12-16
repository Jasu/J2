#pragma once

#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"
#include "containers/trees/rope_tree_algo.hpp"

namespace j::strings::inline unicode::inline rope {
  template<bool IsConst>
  u64_t basic_rope_utf8_byte_iterator<IsConst>::position() const noexcept {
    J_ASSUME(m_char);
    return m_it.cumulative_metrics().key()
      + (m_char - m_it->text.begin());
  }

  template<bool IsConst>
  void basic_rope_utf8_byte_iterator<IsConst>::rewind(u64_t num) noexcept {
    const char * begin = m_it->text.begin();
    J_ASSUME_NOT_NULL(m_char);
    while (num > (u64_t)(m_char - begin)) {
      num -= m_char - begin;
      if (J_UNLIKELY(!m_it.has_previous())) {
        J_ASSUME(num == 1);
        break;
      }
      auto text = (--m_it)->text;
      begin = text.begin();
      m_end = m_char = text.end();
    }

    m_char -= num;
  }

  template<bool IsConst>
  void basic_rope_utf8_byte_iterator<IsConst>::forward(u64_t num) noexcept {
    while (num >= (u64_t)(m_end - m_char)) {
      num -= m_end - m_char;
      if (J_UNLIKELY(!m_it.has_next())) {
        m_char = m_end;
        J_ASSUME(!num);
        return;
      }
      ++m_it;
      const auto text = m_it->text;
      m_char = text.begin();
      m_end = text.end();
    }

    m_char += num;
  }

  template<bool IsConst>
  i64_t operator-(const basic_rope_utf8_byte_iterator<IsConst> & lhs,
                  const basic_rope_utf8_byte_iterator<IsConst> & rhs) noexcept
  {
    return containers::trees::rope_value_distance(*lhs.m_it, *rhs.m_it)
      + (lhs.m_char - lhs.m_it->text.begin())
      - (rhs.m_char - rhs.m_it->text.begin());
  }
}
