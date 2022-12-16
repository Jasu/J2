#include "strings/unicode/rope/rope_utf8_view.hpp"

namespace j::strings::inline unicode::inline rope {
  namespace unicode_rope_detail {
    string interval_to_string(const_rope_utf8_byte_iterator begin,
                              const_rope_utf8_byte_iterator end)
    {
      const u32_t begin_pos = begin.position(), end_pos = end.position();
      J_ASSERT(begin_pos <= end_pos, "Unordered iterators.");
      string result(end_pos - begin_pos);
      char * write_ptr = result.data();

      const char *begin_ptr = begin.m_char, *end_ptr = begin.m_end;
      auto it = begin.m_it;
      while (it != end.m_it) {
        if (begin_ptr != end_ptr) {
          ::j::memcpy(write_ptr, begin_ptr, end_ptr - begin_ptr);
          write_ptr += end_ptr - begin_ptr;
        }
        if (++it) {
          auto txt = it->text;
          begin_ptr = txt.begin();
          end_ptr = txt.end();
        } else {
          J_ASSERT(it == end.m_it, "Out of range");
          return result;
        }
      }
      end_ptr = end.m_end;
      if (begin_ptr != end_ptr) {
        ::j::memcpy(write_ptr, begin_ptr, end_ptr - begin_ptr);
      }

      return result;
    }
  }
}
