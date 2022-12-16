#include <detail/preamble.hpp>

#include "properties/enum_registration.hpp"
#include "properties/access.hpp"
#include "strings/parsing/parse.hpp"
#include "strings/format.hpp"

TEST_SUITE_BEGIN("Properties - Enum Access");

using namespace j::properties;

namespace {
  enum class enum_1 {
    value_1 = 1,
    value_2 = 9,
    value_3 = 3,
  };

  enum_access_registration<enum_1> enum_1_reg{
    "enum_1",
    value = enum_value(enum_1::value_1, "value_1"),
    value = enum_value(enum_1::value_2, "value_2"),
    value = enum_value(enum_1::value_3, "value_3"),
  };
}

TEST_CASE("Can construct empty") {
  auto e = enum_1::value_1;
  enum_access a(e);
}

TEST_CASE("Can read u64_t") {
  auto e = enum_1::value_2;
  enum_access a(e);
  REQUIRE(a.value() == 9);
}

TEST_CASE("Can read enum") {
  auto e = enum_1::value_2;
  enum_access a(e);
  REQUIRE(a.value<enum_1>() == enum_1::value_2);
}

TEST_CASE("Can write enum") {
  auto e = enum_1::value_2;
  enum_access a(e);
  a.set_value(enum_1::value_3);
  REQUIRE(e == enum_1::value_3);
}

TEST_CASE("Can access via wrapper") {
  auto e = enum_1::value_1;
  auto w = access_property(e);
  REQUIRE(w.is_enum());
  REQUIRE(w.type() == PROPERTY_TYPE::ENUM);
  REQUIRE(w.as_enum().value() == 1);
}

TEST_CASE("Can parse from string") {
  enum_1 e = j::strings::parse_fn<enum_1>::parse("value_2");
  REQUIRE(e == enum_1::value_2);
}

TEST_CASE("Can format to string") {
  enum_1 e = enum_1::value_2;
  REQUIRE(j::strings::format("{}", e) == "value_2");
}

TEST_SUITE_END();
