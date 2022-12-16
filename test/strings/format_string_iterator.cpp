#include <detail/preamble.hpp>

#include "strings/formatting/format_string_iterator.hpp"

TEST_SUITE_BEGIN("Strings - Format String Iterator");

using namespace j::strings;

namespace {
  void assert_type(format_string_token type, const format_string_iterator & it) {
    REQUIRE_UNARY(it.token_type() == type);
    REQUIRE_UNARY(it.is_plain_text() == (type == format_string_token::plain_text));
    REQUIRE_UNARY(it.is_placeholder() == (type == format_string_token::placeholder));
    REQUIRE_UNARY(it.is_style_push() == (type == format_string_token::style_push));
    REQUIRE_UNARY(it.is_style_pop() == (type == format_string_token::style_pop));
    REQUIRE_UNARY(it.is_end() == (type == format_string_token::end));
    REQUIRE_UNARY(!(bool)it == (type == format_string_token::end));
    REQUIRE_UNARY(!it == (type == format_string_token::end));
  }
}

TEST_CASE("Empty string") {
  format_string_iterator it("");
  assert_type(format_string_token::end, it);
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Plain text") {
  format_string_iterator it("Hello, world!");
  assert_type(format_string_token::plain_text, it);
  REQUIRE(it.current() == "Hello, world!");
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Empty placeholder") {
  format_string_iterator it("{}");
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{}");
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Placeholder with data") {
  format_string_iterator it("{:hello}");
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{:hello}");
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Style push") {
  format_string_iterator it("{#bold}");
  assert_type(format_string_token::style_push, it);
  REQUIRE(it.current() == "{#bold}");
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Style pop") {
  format_string_iterator it("{/}");
  assert_type(format_string_token::style_pop, it);
  REQUIRE(it.current() == "{/}");
  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Hello, {}!") {
  format_string_iterator it("Hello, {}!");
  assert_type(format_string_token::plain_text, it);
  REQUIRE(it.current() == "Hello, ");

  it.next();
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{}");

  it.next();
  assert_type(format_string_token::plain_text, it);
  REQUIRE(it.current() == "!");

  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("Hello, {#bold}{}{/}!") {
  format_string_iterator it("Hello, {#bold}{}{/}!");
  assert_type(format_string_token::plain_text, it);
  REQUIRE(it.current() == "Hello, ");

  it.next();
  assert_type(format_string_token::style_push, it);
  REQUIRE(it.current() == "{#bold}");

  it.next();
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{}");

  it.next();
  assert_type(format_string_token::style_pop, it);
  REQUIRE(it.current() == "{/}");

  it.next();
  assert_type(format_string_token::plain_text, it);
  REQUIRE(it.current() == "!");

  it.next();
  assert_type(format_string_token::end, it);
}

TEST_CASE("{hello}{world}") {
  format_string_iterator it("{hello}{world}");
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{hello}");

  it.next();
  assert_type(format_string_token::placeholder, it);
  REQUIRE(it.current() == "{world}");

  it.next();
  assert_type(format_string_token::end, it);
}

TEST_SUITE_END();
