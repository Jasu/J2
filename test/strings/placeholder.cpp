#include <detail/preamble.hpp>

#include "strings/formatting/placeholder.hpp"

using namespace j::strings;

TEST_SUITE_BEGIN("Strings - Placeholder");

TEST_CASE("Empty placeholder") {
  placeholder p("{}");
  REQUIRE(p.formatter_name.empty());
  REQUIRE(p.formatter_options.empty());
}

TEST_CASE("Named placeholder") {
  placeholder p("{hello}");
  REQUIRE(p.formatter_name == "hello");
  REQUIRE(p.formatter_options.empty());
}

TEST_CASE("Named placeholder with empty options") {
  placeholder p("{hello:}");
  REQUIRE(p.formatter_name == "hello");
  REQUIRE(p.formatter_options.empty());
}

TEST_CASE("Placeholder with options") {
  placeholder p("{:hello}");
  REQUIRE(p.formatter_name.empty());
  REQUIRE(p.formatter_options == "hello");
}

TEST_CASE("Named placeholder with options") {
  placeholder p("{hello:world}");
  REQUIRE(p.formatter_name == "hello");
  REQUIRE(p.formatter_options == "world");
}

TEST_SUITE_END();
