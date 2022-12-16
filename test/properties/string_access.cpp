#include <detail/preamble.hpp>

#include "properties/access/string_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "strings/string_view.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "hzd/string.hpp"

TEST_CASE("strings::string string access read") {
  j::strings::string s{"foo"};
  j::properties::access::string_access a{s};
  REQUIRE(a.size() == 3);
  REQUIRE(a.as_j_string() == "foo");
  REQUIRE(a.as_j_string_view() == "foo");
  REQUIRE(j::memcmp(a.data(), "foo", 3) == 0);
}

TEST_CASE("string variant access read") {
  j::properties::property_variant v = "foo";
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::STRING);
  REQUIRE(w.is_string());
  REQUIRE(w.as_string().size() == 3);
  REQUIRE(w.as_string().as_j_string() == "foo");
  REQUIRE(w.as_string().as_j_string_view() == "foo");
  REQUIRE(j::memcmp(w.get_value().as_string().data(), "foo", 3) == 0);
}
