#include "lbr.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8 Line-breaking");

namespace s = j::strings;

TEST_CASE("ASCII no breaks") {
  s::utf8_rope r;
  r += "HelloWorld";
  assert_line_breaks(r, {!br("HelloWorld")});
}

TEST_CASE("ASCII after space breaks") {
  s::utf8_rope r;
  r += "Hello World";
  assert_line_breaks(r, {br("Hello "), !br("World")});
}

TEST_CASE("ASCII newline") {
  s::utf8_rope r;
  r += "Hello\nWorld";
  assert_line_breaks(r, {!br("Hello\n"), !br("World")});
}

TEST_CASE("ASCII CRLF") {
  s::utf8_rope r;
  r += "Hello\r\nWorld";
  assert_line_breaks(r, {!br("Hello\r\n"), !br("World")});
}

TEST_CASE("ASCII CR") {
  s::utf8_rope r;
  r += "Hello\rWorld";
  assert_line_breaks(r, {!br("Hello\r"), !br("World")});
}

TEST_SUITE_END();
