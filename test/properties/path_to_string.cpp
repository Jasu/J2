#include <detail/preamble.hpp>

#include "properties/path.hpp"
#include "containers/vector.hpp"
#include "strings/string.hpp"
#include "strings/format.hpp"

TEST_SUITE("Properties - Path formatter") {

  using namespace j::properties;
  using namespace j::strings;

  TEST_CASE("to_string(path): \"\"") {
    REQUIRE_UNARY(format("{}", path{}) == "");
  }

  TEST_CASE("to_string(path): [0]") {
    path p;
    p.emplace_back(array_index_tag, 0ULL);
    REQUIRE_UNARY(format("{}", p) == "[0]");
  }

  TEST_CASE("to_string(path): hello") {
    path p;
    p.emplace_back(property_name_tag, "hello");
    REQUIRE_UNARY(format("{}", p) == "hello");
  }

  TEST_CASE("to_string(path): *EMPTY-ERROR*") {
    path p;
    p.emplace_back();
    REQUIRE_UNARY(format("{}", p) == "*EMPTY-ERROR*");
  }

  TEST_CASE("to_string(path): [\"hello\"]") {
    path p;
    p.emplace_back(map_key_tag, "hello");
    REQUIRE_UNARY(format("{}", p) == "[\"hello\"]");
  }

  TEST_CASE("to_string(path): hello[1]") {
    path p;
    p.emplace_back(property_name_tag, "hello");
    p.emplace_back(array_index_tag, 1ULL);
    REQUIRE_UNARY(format("{}", p) == "hello[1]");
  }

  TEST_CASE("to_string(path): [1].hello") {
    path p;
    p.emplace_back(array_index_tag, 1ULL);
    p.emplace_back(property_name_tag, "hello");
    REQUIRE_UNARY(format("{}", p) == "[1].hello");
  }

  TEST_CASE("to_string(path): hello[\"world\"]") {
    path p;
    p.emplace_back(property_name_tag, "hello");
    p.emplace_back(map_key_tag, "world");
    REQUIRE_UNARY(format("{}", p) == "hello[\"world\"]");
  }
  TEST_CASE("to_string(path): hello.*EMPTY-ERROR*") {
    path p;
    p.emplace_back(property_name_tag, "hello");
    p.emplace_back();
    REQUIRE_UNARY(format("{}", p) == "hello.*EMPTY-ERROR*");
  }
}
