#include "parsing/base_state.hpp"
#include "exceptions/assert.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"
#pragma clang diagnostic ignored "-Wreturn-type"

/*!rules:re2c:common
  re2c:api:style             = free-form;
  re2c:define:YYCTYPE        = "char";
  re2c:define:YYCURSOR       = "cursor";
  re2c:define:YYLIMIT        = "limit";
  re2c:define:YYMARKER       = "marker";
  re2c:define:YYFILL         = "(void)0;";

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;
*/
namespace j::parsing {
  J_A(NORET,COLD) void base_state::fail(strings::const_string_view msg) const {
    J_THROW("{} in {} on line {} at col {}.",  msg, buffer_name, line_number, 1U + cursor - line_begin);
  }
  J_A(NORET,COLD) void base_state::unexpected(strings::const_string_view ctx) const {
    unexpected(ctx, *cursor);
  }
  J_A(NORET,COLD) void base_state::unexpected(strings::const_string_view ctx, char c) const {
    strings::const_string_view ch{&c, 1U};
    const char * delim = "";
    switch (c) {
    case 0:    ch = "end of file"; break;
    case '\n': ch = "newline"; break;
    case '\r': ch = "carriage return"; break;
    case ' ':  ch = "space"; break;
    case '\t': ch = "tab"; break;
    default:   delim = "'"; break;
    }
    J_THROW("Unexpected {}{}{} {} in {} on line {} at col {}.", delim, ch, delim, ctx, buffer_name, line_number, 1U + cursor - line_begin);
  }

  i64_t base_state::parse_int() {
    const char * begin = cursor;
    /*!local:re2c:int
      !use:common;
      [-]? "0b" [01]+ {
        return parse_bin(begin);
      }
      [-]? "0x" [0-9a-fA-F]+ {
        return parse_hex(begin);
      }
      [-]? [0-9]+ {
        return parse_dec(begin);
      }
      * { goto err; }
     */
  err:
    unexpected("when parsing an integer");
  }

  i64_t base_state::parse_dec() {
    const char * begin = cursor;
    /*!local:re2c:dec
      !use:common;
      [-]? [0-9]+ {
        return parse_dec(begin);
      }
      * { goto err; }
     */
  err:
    unexpected("when parsing a decimal integer");
  }

  i64_t base_state::parse_bin() {
    const char * begin = cursor;
    /*!local:re2c:bin
      !use:common;
      [-]? "0b" [01]+ {
        return parse_bin(begin);
      }
      * { goto err; }
     */
  err:
    unexpected("when parsing a binary integer");
  }

  i64_t base_state::parse_hex() {
    const char * begin = cursor;
    /*!local:re2c:hex
      !use:common;
      [-]? "0x" [0-9a-fA-F]+ {
        return parse_hex(begin);
      }
      * { goto err; }
     */
  err:
    unexpected("when parsing a hex integer");
  }

  i64_t base_state::parse_int(const char * J_NOT_NULL begin) const {
    const char * c = begin + (*begin == '-');
    if (*c == '0') {
      if (c[1] == 'x') {
        return parse_hex(begin);
      } else if (c[1] == 'b') {
        return parse_bin(begin);
      }
    }
    return parse_dec(begin);
  }

  i64_t base_state::parse_dec(const char * J_NOT_NULL begin) const {
    i64_t sign = 1;
    if (*begin == '-') {
      sign = -1;
      ++begin;
    }
    i64_t result = 0;
    for (; begin != cursor; ++begin) {
      result *= 10;
      check(*begin >= '0' && *begin <= '9', "Unexpected character in a decimal integer");
      result += *begin - '0';
    }
    return result * sign;
  }

  i64_t base_state::parse_bin(const char * J_NOT_NULL begin) const {
    i64_t sign = 1;
    if (*begin == '-') {
      sign = -1;
      ++begin;
    }
    begin += 2U;
    check(begin[0] == '0' && begin[1] == 'b', "Unexpected prefix in a binary integer");
    i64_t result = 0;
    for (; begin != cursor; ++begin) {
      result <<= 1;
      check(*begin == '0' || *begin == '1', "Unexpected character in a binary integer");
      result += *begin - '0';
    }
    return result * sign;
  }

  i64_t base_state::parse_hex(const char * J_NOT_NULL begin) const {
    i64_t sign = 1;
    if (*begin == '-') {
      sign = -1;
      ++begin;
    }
    check(begin[0] == '0' && begin[1] == 'x', "Unexpected prefix in a hex integer");
    begin += 2U;
    i64_t result = 0;
    for (; begin != cursor; ++begin) {
      result <<= 4;
      char c = *begin;
      check((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'), "Unexpected character in a hex integer");
      result += c > '9' ? (c | 0x20) - 'a' + 0xA : c - '0';
    }
    return result * sign;
  }
}
