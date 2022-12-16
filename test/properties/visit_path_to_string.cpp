#include <detail/preamble.hpp>

#include "properties/visiting/visit_path.hpp"
#include "strings/format.hpp"

using namespace j::properties;
using namespace j::properties::visiting;
using namespace j::strings;

TEST_SUITE_BEGIN("Properties - Visit path formatter");

TEST_CASE("to_string(visit_path): \"\"") {
  REQUIRE(format("{}", visit_path::root) == "");
}

TEST_CASE("to_string(visit_path): [0]") {
  visit_path p{COMPONENT_TYPE::ARRAY_INDEX, visit_path::root};
  p.set_array_index(0);
  REQUIRE(format("{}", p) == "[0]");
}

TEST_CASE("to_string(visit_path): hello") {
  visit_path p{COMPONENT_TYPE::PROPERTY_NAME, visit_path::root};
  p.set_string("hello");
  REQUIRE(format("{}", p) == "hello");
}

TEST_CASE("to_string(visit_path): [\"hello\"]") {
  visit_path p{COMPONENT_TYPE::MAP_KEY, visit_path::root};
  p.set_string("hello");
  REQUIRE(format("{}", p) == "[\"hello\"]");
}

TEST_CASE("to_string(visit_path): hello[1]") {
  visit_path p{COMPONENT_TYPE::PROPERTY_NAME, visit_path::root};
  p.set_string("hello");
  visit_path p2{COMPONENT_TYPE::ARRAY_INDEX, p};
  p2.set_array_index(1);
  REQUIRE(format("{}", p2) == "hello[1]");
}

TEST_CASE("to_string(visit_path): [1].hello") {
  visit_path p{COMPONENT_TYPE::ARRAY_INDEX, visit_path::root};
  p.set_array_index(1);
  visit_path p2{COMPONENT_TYPE::PROPERTY_NAME, p};
  p2.set_string("hello");
  REQUIRE(format("{}", p2) == "[1].hello");
}

TEST_CASE("to_string(visit_path): hello[\"world\"]") {
  visit_path p{COMPONENT_TYPE::PROPERTY_NAME, visit_path::root};
  p.set_string("hello");
  visit_path p2{COMPONENT_TYPE::MAP_KEY, p};
  p2.set_string("world");
  REQUIRE(format("{}", p2) == "hello[\"world\"]");
}

TEST_SUITE_END();
