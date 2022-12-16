#pragma once

#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings::inline unicode::inline rope {
  inline void validate_iterator(const_rope_utf8_byte_iterator it) {
    if (!it.m_it) {
      J_ASSERT(it.m_char == it.m_end);
      return;
    }
    J_ASSERT(it.m_char <= it.m_end, "End < It");
    J_ASSERT(it.m_char >= it.m_it->text.begin() && it.m_char <= it.m_it->text.end(),
             "Character iterator is not in in range.");
    J_ASSERT(it.m_end >= it.m_it->text.begin() && it.m_end <= it.m_it->text.end(),
             "End iterator is not in in range.");
    J_ASSERT(it.m_it->text.end() == it.m_end, "End does not match chunk end");
 }
}
