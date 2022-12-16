#include <detail/preamble.hpp>

#include "colors/rgb.hpp"
#include "properties/access.hpp"
#include "properties/property_variant.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/vector.hpp"

using namespace j::properties;
using namespace j::colors;

TEST_CASE("rgb object access definition") {
  rgb8 v;
  auto a = access_property(v);
  REQUIRE(a.is_object());
  auto & props = a.as_object().get_properties();
  REQUIRE(props.size() == 3);
  REQUIRE(props.find("r") != props.end());
  REQUIRE(props.find("g") != props.end());
  REQUIRE(props.find("b") != props.end());
  auto & r = props.at("r"), & g = props.at("g"), & b = props.at("b");
  REQUIRE(r.type_id() == typeid(u8_t));
  REQUIRE(g.type_id() == typeid(u8_t));
  REQUIRE(b.type_id() == typeid(u8_t));
}

TEST_CASE("rgb object access read") {
  rgb8 v = { 11, 22, 33 };
  auto a = access_property(v).as_object();
  REQUIRE(a.get_property("r").as_int().get_value() == 11);
  REQUIRE(a.get_property("g").as_int().get_value() == 22);
  REQUIRE(a.get_property("b").as_int().get_value() == 33);
}

TEST_CASE("rgb object access write") {
  rgb8 v = { 11, 22, 33 };
  auto a = access_property(v).as_object();

  REQUIRE(v.r == 11);
  a.get_property("r").as_int().set_value(77);
  REQUIRE(v.r == 77);

  REQUIRE(v.g == 22);
  a.get_property("g").as_int().set_value(88);
  REQUIRE(v.g == 88);

  REQUIRE(v.b == 33);
  a.get_property("b").as_int().set_value(99);
  REQUIRE(v.b == 99);
}

TEST_CASE("rgb object access from string") {
  auto def = access::registry::get_typed_access_definition<rgb8>();
  auto c = def->create_from("#F7A890");
  REQUIRE(c.is_object());
  CHECK(c.as_object().get_property("r").as_int().get_value() == 0xF7);
  CHECK(c.as_object().get_property("g").as_int().get_value() == 0xA8);
  CHECK(c.as_object().get_property("b").as_int().get_value() == 0x90);

  auto c2 = def->create_from("#abcdef");
  REQUIRE(c2.is_object());
  CHECK(c2.as_object().get_property("r").as_int().get_value() == 0xAB);
  CHECK(c2.as_object().get_property("g").as_int().get_value() == 0xCD);
  CHECK(c2.as_object().get_property("b").as_int().get_value() == 0xEF);
}

TEST_CASE("rgb object access from short string") {
  auto def = access::registry::get_typed_access_definition<rgb8>();

  auto c = def->create_from("#1FA");
  REQUIRE(c.is_object());
  CHECK(c.as_object().get_property("r").as_int().get_value() == 0x11);
  CHECK(c.as_object().get_property("g").as_int().get_value() == 0xFF);
  CHECK(c.as_object().get_property("b").as_int().get_value() == 0xAA);

  auto c2 = def->create_from("#abf");
  REQUIRE(c2.is_object());
  CHECK(c2.as_object().get_property("r").as_int().get_value() == 0xAA);
  CHECK(c2.as_object().get_property("g").as_int().get_value() == 0xBB);
  CHECK(c2.as_object().get_property("b").as_int().get_value() == 0xFF);
}

TEST_CASE("rgb object access from int array") {
  list_t l;
  l.emplace_back(0x81);
  l.emplace_back(0x70);
  l.emplace_back(0x11);
  auto def = access::registry::get_typed_access_definition<rgb8>();
  auto c = def->create_from(l);
  REQUIRE(c.is_object());
  CHECK(c.as_object().get_property("r").as_int().get_value() == 0x81);
  CHECK(c.as_object().get_property("g").as_int().get_value() == 0x70);
  CHECK(c.as_object().get_property("b").as_int().get_value() == 0x11);
}

TEST_CASE("rgb object access from float array") {
  list_t l;
  l.emplace_back(0.5);
  l.emplace_back(0.0);
  l.emplace_back(1.0);
  auto def = access::registry::get_typed_access_definition<rgb8>();
  auto c = def->create_from(l);
  REQUIRE(c.is_object());
  CHECK(c.as_object().get_property("r").as_int().get_value() == 0x7F);
  CHECK(c.as_object().get_property("g").as_int().get_value() == 0x00);
  CHECK(c.as_object().get_property("b").as_int().get_value() == 0xFF);
}
