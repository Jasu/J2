#pragma once

#include "strings/find.hpp"
#include "strings/string_view.hpp"
#include "strings/string_algo.hpp"

namespace j::strings {
  inline namespace formatting {
    enum class format_string_token : u8_t {
      plain_text,
      placeholder,
      style_push,
      style_pop,
      end,
    };

    class format_string_iterator {
    public:
      explicit inline format_string_iterator(const_string_view format_string) noexcept
        : cur_end(format_string.begin()),
          end(format_string.end())
      {
        next();
      }

      J_A(AI,ND,NODISC) inline format_string_token token_type() const noexcept {
        return m_state;
      }

      J_A(AI,ND,NODISC) inline bool is_plain_text() const noexcept {
        return m_state == format_string_token::plain_text;
      }

      J_A(AI,ND,NODISC) inline bool is_placeholder() const noexcept {
        return m_state == format_string_token::placeholder;
      }

      J_A(AI,ND,NODISC) inline bool is_style_push() const noexcept {
        return m_state == format_string_token::style_push;
      }

      J_A(AI,ND,NODISC) inline bool is_style_pop() const noexcept {
        return m_state == format_string_token::style_pop;
      }


      J_A(AI,ND,NODISC) inline bool is_end() const noexcept {
        return m_state == format_string_token::end;
      }

      J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept {
        return m_state != format_string_token::end;
      }

      J_A(AI,ND,NODISC) inline bool operator!() const noexcept {
        return m_state == format_string_token::end;
      }

      J_A(AI,ND,NODISC) inline strings::const_string_view current() const noexcept {
        return {cur_begin, cur_end};
      }

      void next() {
        cur_begin = cur_end;
        switch (*cur_begin) {
        case '\0':
          m_state = format_string_token::end;
          cur_end = end;
          break;
        case '{':
          m_state = format_string_token::placeholder;
          switch (cur_begin[1]) {
          case '}':
            cur_end = cur_begin + 2;
            break;
          case '/':
            m_state = format_string_token::style_pop;
            cur_end = cur_begin + 3;
            break;
          case '#':
            m_state = format_string_token::style_push;
            [[fallthrough]];
          default:
            cur_end = j::memchr(cur_begin, '}', end - cur_begin) + 1;
            break;
          }
          break;
        case '\\':
          ++cur_begin;
          [[fallthrough]];
        default:
          m_state = format_string_token::plain_text;
          cur_end = cur_begin + 1 + j::strcspn(cur_begin + 1, "{\\");
          break;
        }
      }
    private:
      format_string_token m_state = format_string_token::plain_text;
      const char *cur_begin, *cur_end, *end;
    };
  }
}
