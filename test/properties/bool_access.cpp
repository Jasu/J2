#include <detail/preamble.hpp>

#include "properties/access/bool_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"

TEST_CASE("bool_t access read") {
  j::properties::bool_t i = true;
  j::properties::access::bool_access a{i};
  REQUIRE(a.get_value() == true);
}

TEST_CASE("bool_t wrapper get_value") {
  j::properties::bool_t i = true;
  auto w = j::properties::wrappers::access(i);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::BOOL);
  REQUIRE(w.is_bool());
  REQUIRE(w.as_bool().get_value() == true);
}

TEST_CASE("bool_t wrapper set_value") {
  j::properties::bool_t i = true;
  auto w = j::properties::wrappers::access(i);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::BOOL);
  REQUIRE(w.is_bool());
  w.as_bool().set_value(false);
  REQUIRE(i == false);
}

TEST_CASE("bool_t access set_value") {
  j::properties::bool_t i = true;
  j::properties::access::bool_access a{i};
  REQUIRE(a.get_value() == true);
  a.set_value(false);
  REQUIRE(a.get_value() == false);
  REQUIRE(i == false);
}

TEST_CASE("variant bool_t access read") {
  j::properties::property_variant v = true;
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::BOOL);
  REQUIRE(w.is_bool());
  REQUIRE(w.as_bool().get_value() == true);
}

TEST_CASE("variant bool_t access set_value") {
  j::properties::property_variant v = true;
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::BOOL);
  REQUIRE(w.is_bool());
  w.as_bool().set_value(false);
  REQUIRE(w.as_bool().get_value() == false);
  REQUIRE(v.as<j::properties::bool_t>() == false);
}
