#include <detail/preamble.hpp>

#include "properties/access.hpp"
#include "properties/visiting/visit.hpp"
#include "properties/property_variant.hpp"
#include "properties/classes/object_access_registration.hpp"

using namespace j::properties;
using namespace j::properties::access;
using namespace j::properties::visiting;
using namespace j::properties::classes;

TEST_CASE("Visit nil") {
  auto nil = nullptr;
  typed_access a{nil};
  REQUIRE(visit(a, [](auto & access) -> int {
    REQUIRE(access.type() == PROPERTY_TYPE::NIL);
    REQUIRE(access.is_nil());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, nil_access>) {
      return 4;
    } else {
      REQUIRE(false);
      return 0;
    }
  }) == 4);
}

TEST_CASE("Visit string") {
  j::strings::string s = "foo";
  typed_access a{s};
  REQUIRE(visit(a, [](auto & access) -> j::strings::string {
    REQUIRE(access.type() == PROPERTY_TYPE::STRING);
    REQUIRE(access.is_string());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, string_access>) {
      return "hello" + access.as_j_string();
    } else {
      REQUIRE(false);
      return "";
    }
  }) == "hellofoo");
}

TEST_CASE("Visit int") {
  int i = 9999;
  typed_access a{i};
  REQUIRE(visit(a, [](auto & access) -> int {
    REQUIRE(access.type() == PROPERTY_TYPE::INT);
    REQUIRE(access.is_int());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, int_access>) {
      return access.get_value() + 1;
    } else {
      REQUIRE(false);
      return 0;
    }
  }) == 10000);
}

TEST_CASE("Visit bool") {
  bool b = false;
  typed_access a{b};
  REQUIRE(visit(a, [](auto & access) -> bool {
    REQUIRE(access.type() == PROPERTY_TYPE::BOOL);
    REQUIRE(access.is_bool());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, bool_access>) {
      return !access.get_value();
    } else {
      REQUIRE(false);
      return false;
    }
  }) == true);
}

TEST_CASE("Visit float") {
  float f = 1.999f;
  typed_access a{f};
  REQUIRE(visit(a, [](auto & access) -> float {
    REQUIRE(access.type() == PROPERTY_TYPE::FLOAT);
    REQUIRE(access.is_float());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, float_access>) {
      return -access.get_value();
    } else {
      REQUIRE(false);
      return false;
    }
  }) == -1.999f);
}

TEST_CASE("Visit list") {
  list_t l;
  typed_access a{l};
  REQUIRE(visit(a, [](auto & access) -> sz_t {
    REQUIRE(access.type() == PROPERTY_TYPE::LIST);
    REQUIRE(access.is_list());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, list_access>) {
      return access.size();
    } else {
      REQUIRE(false);
      return 1;
    }
  }) == 0);
}

TEST_CASE("Visit map") {
  map_t m;
  typed_access a{m};
  REQUIRE(visit(a, [](auto & access) -> sz_t {
    REQUIRE(access.type() == PROPERTY_TYPE::MAP);
    REQUIRE(access.is_map());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, map_access>) {
      return access.size();
    } else {
      REQUIRE(false);
      return 1;
    }
  }) == 0);
}

struct J_TYPE_HIDDEN foo {
  int x;
};

static object_access_registration<foo> foo_reg{
  "foo",
  property = member<&foo::x>("x"),
};

TEST_CASE("Visit object") {
  foo f;
  typed_access a{f};
  f.x = -919;
  REQUIRE(visit(a, [](auto & access) -> int {
    REQUIRE(access.type() == PROPERTY_TYPE::OBJECT);
    REQUIRE(access.is_object());
    if constexpr (j::is_same_v<j::decay_t<decltype(access)>, object_access>) {
      return access.get_property("x").as_int().get_value();
    } else {
      REQUIRE(false);
      return 1;
    }
  }) == -919);
}
