#pragma once

#include "strings/string_view.hpp"
#include "strings/unicode/utf8.hpp"
#include "format_digits.hpp"

namespace j::strings::inline formatting {
  struct escape_8bit final {
    char input;
    const_string_view escape;
  };

  inline constexpr escape_8bit default_escape_table[]{
    { '\n', "\\n" },
    { '\r', "\\r" },
    { '\a', "\\a" },
    { '\b', "\\b" },
    { '\f', "\\f" },
    { '\t', "\\t" },
    { '\v', "\\v" },
    { '\0', "\\0" },
  };

  inline constexpr escape_8bit unicode_escape_table[]{
    { '\n', "␊" },
    { '\r', "␍" },
    { '\a', "␇" },
    { '\b', "␈" },
    { '\f', "␌" },
    { '\t', "␉" },
    { '\v', "␋" },
    { '\0', "␀" },
  };

  struct escape_options final {
    const_string_view hex_escape_starter;
    const_string_view unicode_bmp_escape_starter;
    const_string_view unicode_full_escape_starter;
    u32_t num_escapes;
    const escape_8bit * escapes;
  };

  inline constexpr escape_options default_escape_options{
    "\\x",
    "\\u",
    "\\U",
    J_ARRAY_SIZE(default_escape_table),
    default_escape_table
  };

  inline constexpr escape_options unicode_escape_options{
    "🆇 ",
    "🆄 ",
    "🆄 ",
    J_ARRAY_SIZE(default_escape_table),
    default_escape_table
  };

  enum escape_type {
    none,
    hex,
    unicode,
    special,
  };

  template<typename Fn>
  void iterate_escaped(const_string_view s, Fn && callback, const escape_options & opts = default_escape_options) {
    auto unescaped_begin = s.begin();
    auto it = s.begin();
    const auto end = s.end();
    char escape_buffer[11];
    while (it != end) {
      J_ASSERT(it < end, "Iterated out of range.");
      if ((u8_t)*it >= 0x80U) {
        u32_t in_size = 0;
        u32_t out_size = 0;
        escape_type type = escape_type::none;
        if (!utf8_is_code_point_valid(it, end - it)) {
          out_size = opts.hex_escape_starter.size();
          ::j::memcpy(escape_buffer, opts.hex_escape_starter.begin(), out_size);
          format_hex(*it, escape_buffer + out_size, 2, 'A' - 10);
          in_size = 1U;
          type = escape_type::hex;
          out_size += 2U;
        } else {
          const u32_t code_point = utf8_get_code_point(it);
          if (code_point > 0xFFFFU) {
            out_size = opts.unicode_full_escape_starter.size();
            ::j::memcpy(escape_buffer, opts.unicode_full_escape_starter.begin(), out_size);
            format_hex(code_point, escape_buffer + out_size, 6, 'A' - 10);
            out_size += 6;
          } else {
            out_size = opts.unicode_bmp_escape_starter.size();
            ::j::memcpy(escape_buffer, opts.unicode_bmp_escape_starter.begin(), out_size);
            format_hex(code_point, escape_buffer + out_size, 4, 'A' - 10);
            out_size += 4U;
          }
          type = escape_type::unicode;
          in_size = utf8_code_point_bytes(it);
        }
        if (it != unescaped_begin) {
          J_ASSERT(unescaped_begin < it, "Iterated out of range.");
          static_cast<Fn &&>(callback)(escape_type::none, const_string_view(unescaped_begin, it));
        }
        static_cast<Fn &&>(callback)(type, const_string_view(escape_buffer, out_size));
        it += in_size;
        unescaped_begin = it;
        continue;
      }

      for (u32_t i = 0U; i < opts.num_escapes; ++i) {
        if (opts.escapes[i].input == *it) {
          if (it != unescaped_begin) {
            J_ASSERT(unescaped_begin < end, "Iterated out of range.");
            static_cast<Fn &&>(callback)(escape_type::none, const_string_view(unescaped_begin, it));
          }
          static_cast<Fn &&>(callback)(escape_type::special, opts.escapes[i].escape);
          ++it;
          unescaped_begin = it;
          goto next;
        }
      }

      ++it;
    next: ;
    }
    if (it != unescaped_begin) {
      J_ASSERT(unescaped_begin < end, "Iterated out of range.");
      static_cast<Fn &&>(callback)(escape_type::none, const_string_view(unescaped_begin, it));
    }
  }

  [[nodiscard]] inline u32_t count_escaped_bytes(const_string_view s, const escape_options & opts = default_escape_options) noexcept {
    u32_t result = 0U;
    iterate_escaped(s, [&result](escape_type, const_string_view s) noexcept {
      J_ASSERT_NOT_NULL(s);
      result += s.size();
    }, opts);
    return result;
  }
}
