#pragma once

#include "util/hex.hpp"
#include "strings/string.hpp"
#include "strings/unicode/utf8.hpp"

namespace j::strings::inline parsing {
  struct escape_8bit final {
    char escape_character;
    char output;
  };

  inline constexpr escape_8bit default_escape_table[]{
    { 'n', '\n' },
    { 'r', '\r' },
    { 'a', '\a' },
    { 'b', '\b' },
    { 'f', '\f' },
    { 't', '\t' },
    { 'v', '\v' },
  };

  struct escape_options final {
    char escape_character = '\\';

    char hex_byte_escape_character = 'x';
    u8_t utf8_bmp_escape_character = 'u';
    u8_t utf8_full_escape_character = 'U';

    u32_t num_escapes = sizeof(default_escape_table) / sizeof(escape_8bit);
    const escape_8bit * escapes = default_escape_table;
  };

  inline constexpr escape_options default_escape_options{};

  inline u32_t unescaped_length(const_string_view str, const escape_options & opts = default_escape_options) {
    u32_t result = str.size();
    for (auto it = str.begin(), end = str.end(); it != end; ++it) {
      if (*it == opts.escape_character) {
        if (J_UNLIKELY(++it == str.end())) {
          J_THROW("Unexpected end of string after escape.");
        }
        if (*it == opts.hex_byte_escape_character) {
          it += 2U;
          J_REQUIRE(it + 1U <= end, "Unexpected end of hex escape");
          result -= 3U;
        } else if (*it == opts.utf8_bmp_escape_character) {
          ++it;
          J_REQUIRE(it + 4U <= end, "Unexpected end of UTF-8 BMP escape");
          J_REQUIRE(util::is_hex_digit(it[0]) && util::is_hex_digit(it[1])
                    && util::is_hex_digit(it[2]) && util::is_hex_digit(it[3]),
                    "Invalid UTF-8 BMP escape.");
          result += utf8_code_point_bytes((util::convert_hex_byte(it) << 8U) | util::convert_hex_byte(it + 2));
          it += 3;
          result -= 6U;
        } else if (*it == opts.utf8_full_escape_character) {
          ++it;
          J_REQUIRE(it + 6U <= end, "Unexpected end of UTF-8 full escape");
          J_REQUIRE(util::is_hex_digit(it[0]) && util::is_hex_digit(it[1])
                    && util::is_hex_digit(it[2]) && util::is_hex_digit(it[3])
                    && util::is_hex_digit(it[4]) && util::is_hex_digit(it[5]) ,
                    "Invalid UTF-8 full escape.");
          result += utf8_code_point_bytes((util::convert_hex_byte(it) << 16U)
                                  | (util::convert_hex_byte(it + 2) << 8U)
                                  | util::convert_hex_byte(it + 4));
          it += 5;
          result -= 8U;
        } else {
          result -= 1U;
        }
      }
    }

    return result;
  }

  inline string parse_escaped(const_string_view str,
                              const escape_options & opts = default_escape_options)
  {
    string result(unescaped_length(str, opts));
    char * write_ptr = result.begin();
    for (auto it = str.begin(), end = str.end();
         it != end; ++it)
    {
      if (*it == opts.escape_character) {
        ++it;
        if (J_UNLIKELY(it == str.end())) {
          J_THROW("Unexpected end of string after escape.");
        }
        if (*it == opts.hex_byte_escape_character) {
          ++it;
          J_REQUIRE(util::is_hex_digit(it[0]) && util::is_hex_digit(it[1]), "Invalid hex escape.");
          *write_ptr++ = util::convert_hex_byte(it++);
          continue;
        } else if (*it == opts.utf8_bmp_escape_character) {
          ++it;
          write_ptr = utf8_encode(write_ptr, (util::convert_hex_byte(it) << 8U) | util::convert_hex_byte(it + 2));
          it += 3;
          continue;
        } else if (*it == opts.utf8_full_escape_character) {
          ++it;
          write_ptr = utf8_encode(write_ptr,
                                  (util::convert_hex_byte(it) << 16U)
                                  | (util::convert_hex_byte(it + 2) << 8U)
                                  | util::convert_hex_byte(it + 4));
          it += 5;
          continue;
        } else {
          for (u32_t i = 0U; i < opts.num_escapes; ++i) {
            if (*it == opts.escapes[i].escape_character) {
              *write_ptr++ = opts.escapes[i].output;
              goto next;
            }
          }
        }
      }
      *write_ptr++ = *it;
    next: ;
    }
    J_ASSERT(write_ptr == result.end(), "Did not fill the string.");
    return result;
  }
}
