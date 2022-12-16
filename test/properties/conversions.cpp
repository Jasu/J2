#include <detail/preamble.hpp>

#include "properties/map_registration.hpp"
#include "properties/class_registration.hpp"
#include "properties/access.hpp"
#include "strings/string.hpp"
#include "strings/string_cmp.hpp"

TEST_SUITE_BEGIN("Properties - Conversions");

using namespace j::properties;

struct my_string {
  int i;
};

namespace {
  static object_access_registration<my_string> my_string_reg(
    "my_string",
    convertible_to = [](const my_string & rhs) { return j::strings::string(rhs.i > 0 ? "hello" : "world"); }
    );
}

TEST_CASE("assign from") {
  j::strings::string s = "Frooba";
  auto sa = access_property(s);
  REQUIRE(sa.is_string());

  my_string hello{1};

  sa.as_string().assign_from(hello);

  REQUIRE(sa.is_string());
  REQUIRE(sa.as_string().as_j_string() == "hello");
  REQUIRE(s == "hello");
}

TEST_SUITE_END();
