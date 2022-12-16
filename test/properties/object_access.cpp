#include <detail/preamble.hpp>

#include "properties/classes/object_access_registration.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "strings/string.hpp"

TEST_SUITE_BEGIN("Properties - Object Access");

using namespace j::properties;
using namespace j::properties::classes;
using namespace j::properties::access;


namespace {
  struct frooba {
    int x;
    bool y;
    float z;
    float & access_z() {
      return z;
    }
  };

  const object_access_registration<frooba> frooba_reg{
    "frooba",
    property = member<&frooba::x>("x"),
    property = member<&frooba::y>("y"),
    property = accessor<&frooba::access_z>("z")
  };
}

TEST_CASE("Property names") {
  frooba froo;
  auto a = wrappers::access(froo);
  REQUIRE(a.is_object());
  auto n = a.as_object().property_names();
  REQUIRE(n.size() == 3);
  REQUIRE(n[0] == "x");
  REQUIRE(n[1] == "y");
  REQUIRE(n[2] == "z");
}

TEST_CASE("Read") {
  frooba froo{
    .x = 4,
    .y = true,
    .z = 1.99f,
  };
  auto a = wrappers::access(froo);
  REQUIRE(a.is_object());
  REQUIRE(a.type() == j::properties::PROPERTY_TYPE::OBJECT);
  auto oa = a.as_object();

  auto xa = oa.as_object().get_property("x");
  REQUIRE(xa.type() == PROPERTY_TYPE::INT);
  REQUIRE(xa.as_int().get_value() == 4);

  auto ya = oa.as_object().get_property("y");
  REQUIRE(ya.type() == PROPERTY_TYPE::BOOL);
  REQUIRE(ya.as_bool().get_value() == true);

  auto za = oa.as_object().get_property("z");
  REQUIRE(za.type() == PROPERTY_TYPE::FLOAT);
  REQUIRE(za.as_float().get_value() == 1.99f);
}

TEST_CASE("Write") {
  frooba froo{
    .x = 4,
    .y = true,
    .z = 1.99f,
  };

  auto pa = wrappers::access(froo);
  REQUIRE(pa.type() == j::properties::PROPERTY_TYPE::OBJECT);

  auto xa = pa.as_object().get_property("x");
  REQUIRE(xa.type() == j::properties::PROPERTY_TYPE::INT);
  xa.as_int().set_value(-9);
  REQUIRE(froo.x == -9);

  auto ya = pa.as_object().get_property("y");
  REQUIRE(ya.type() == j::properties::PROPERTY_TYPE::BOOL);
  ya.as_bool().set_value(false);
  REQUIRE(froo.y == false);

  auto za = pa.as_object().get_property("z");
  REQUIRE(za.type() == j::properties::PROPERTY_TYPE::FLOAT);
  za.as_float().set_value(-199.4f);
  REQUIRE(froo.z == -199.4f);
}

namespace {
  struct nested_object {
    j::strings::string s;
    int i;
  };

  struct parent_object {
    float f;
    nested_object child;
  };

  const object_access_registration<parent_object> parent_reg{
    "parent_object",
    property = member<&parent_object::f>("f"),
    property = by_lambda("s", [](parent_object & o) -> j::strings::string & { return o.child.s; }),
    property = by_lambda("i", [](parent_object & o) -> int & { return o.child.i; })
  };
}

TEST_CASE("Nested read") {
  parent_object o{ 1.25f, { "hello", 17 } };
  auto a = wrappers::access(o);
  REQUIRE(a.is_object());
  auto oa = a.as_object();
  REQUIRE(oa.get_property("f").is_float());
  REQUIRE(oa.get_property("f").as_float().get_value() == 1.25f);
  REQUIRE(oa.get_property("s").is_string());
  REQUIRE(oa.get_property("s").as_string().as_j_string_view() == "hello");
  REQUIRE(oa.get_property("i").is_int());
  REQUIRE(oa.get_property("i").as_int().get_value() == 17);
}

TEST_CASE("Nested write") {
  parent_object o{ 1.25f, { "hello", 17 } };
  auto a = wrappers::access(o);
  auto oa = a.as_object();
  oa.get_property("f").as_float().set_value(1899.f);
  REQUIRE(o.f == 1899.f);
  oa.get_property("s").as_string().assign_from("world");
  REQUIRE(o.child.s == "world");
  oa.get_property("i").as_int().set_value(918);
  REQUIRE(o.child.i == 918);
}

TEST_SUITE_END();
