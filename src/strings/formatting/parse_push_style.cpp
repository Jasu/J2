#include "strings/formatting/parse_push_style.hpp"

#include "strings/string_algo.hpp"
#include "strings/styling/style.hpp"
#include "strings/styling/style_registry.hpp"
#include "strings/string.hpp"
#include "strings/find.hpp"
#include "strings/formatting/format_value.hpp"

namespace j::strings {
  inline namespace formatting {
    namespace {
      enum parse_state {
        top,
        name,
        cond,
      };
      J_A(NI,COLD) const char * parse_cond_style(const char *it, const char *end, style & style, const format_value & param) noexcept {
        ++it;
        end = j::memchr(it, ']', end - it);
        i32_t idx = param.get_inline_type() == inline_type::uint ? param.as_uint() : param.as_sint();
        J_ASSERT(idx >= 0);
        const char *next = j::memchr(it, ',', end - it);
        for (i32_t i = 0; i != idx; ++i) {
          J_ASSERT_NOT_NULL(next);
          it = next + 1;
          next = j::memchr(it, ',', end - it);
        }
        if (!next) { next = end; }
        while (*it == ' ') { ++it; }
        if (it != next) {
          override_style_inline(style, g_style_registry->at({it, next}));
        }
        return end + 1;
      }
    }

    void parse_push_style(const_string_view token, style & base_style, const format_value * params, u32_t & current_param) noexcept {
      const char *it = token.begin() + 2, *name_begin = it;
      for (;;) {
        switch (*it) {
        case ',':
        case ' ':
        case '}':
          if (name_begin != it) {
            override_style_inline(base_style, g_style_registry->at({name_begin, it}));
          }
          if (*it == '}') {
            return;
          }
          name_begin = ++it;
          break;
        case '[':
          it = parse_cond_style(it, token.end(), base_style, params[current_param++]);
          name_begin = it;
          break;
        default:
          ++it;
          break;
        }
      }
    }
  }
}
