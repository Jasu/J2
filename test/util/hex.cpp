#include <detail/preamble.hpp>

#include "util/hex.hpp"

using namespace j::util;

TEST_CASE("util::is_hex_digit") {
  for (int i = 0; i < 256; ++i) {
    switch (i) {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
      REQUIRE(is_hex_digit(i));
      break;
    default:
      REQUIRE(!is_hex_digit(i));
      break;
    }
  }
}

TEST_CASE("util::convert_hex_digit") {
  int i = 0;
  for (char c = '0'; c <= '9'; ++c, ++i) {
    REQUIRE(convert_hex_digit(c) == i);
  }
  for (char c = 'a'; c <= 'f'; ++c, ++i) {
    REQUIRE(convert_hex_digit(c) == i);
  }
  i = 10;
  for (char c = 'A'; c <= 'F'; ++c, ++i) {
    REQUIRE(convert_hex_digit(c) == i);
  }
}

TEST_CASE("util::convert_hex_byte") {
  char buf[3];
  for (int i = 0; i < 256; ++i) {
    unsigned char c = i;
    __builtin_snprintf(buf, 3, "%.2xhh", c);
    buf[2] = 9;
    REQUIRE(convert_hex_byte(buf) == i);
    __builtin_snprintf(buf, 3, "%.2Xhh", c);
    buf[2] = 9;
    REQUIRE(convert_hex_byte(buf) == i);
  }
}
