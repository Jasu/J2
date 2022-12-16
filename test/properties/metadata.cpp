#include <detail/preamble.hpp>

#include "properties/map_registration.hpp"
#include "properties/list_registration.hpp"
#include "properties/class_registration.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"
#include "containers/trivial_array.hpp"

TEST_SUITE_BEGIN("Properties - Metadata");

using namespace j;
using namespace j::properties;
using namespace j::attributes;

struct xx { int x; };
struct yy { int x; };

namespace {
  inline constexpr attribute_definition attr1{
    value_type = type<int>,
    tag = type<struct attr1_tag>,
    is_multiple};

  inline constexpr attribute_definition attr2{
    value_type = type<const char *>,
    tag = type<struct attr2_tag>};


  const object_access_registration<xx> reg(
    "xx",
    property = member<&xx::x>("x", attr1 = 58),
    attr1 = 12,
    attr1 = 44,
    attr2 = "hello");

  const object_access_registration<yy> reg2("yy", property = member<&yy::x>("x"));

  list_access_registration<j::vector<u8_t>> list_reg{
    "vector<u8_t>",
    attr1 = -1,
    value_metadata = metadata(
      attr1 = 999,
      attr1 = 1099,
      attr2 = "listitem")
  };
}

TEST_CASE("no access metadata") {
  auto def = access::registry::get_typed_access_definition<yy>();
  REQUIRE(def);
  REQUIRE(def->is_object());
  REQUIRE(!def->metadata().contains(attr1));
  REQUIRE(!def->metadata().contains(attr2));
  REQUIRE(!def->metadata().contains(property));
}

TEST_CASE("access metadata" * doctest::skip()) {
  auto def = access::registry::get_typed_access_definition<xx>();
  REQUIRE(def);
  REQUIRE(def->is_object());
  REQUIRE(!def->metadata().contains(property));
  REQUIRE(def->metadata().contains(attr1));
  REQUIRE(def->metadata().contains(attr2));
  auto metadata1 = def->metadata().get(attr1);
  REQUIRE(metadata1.size() == 2);
  REQUIRE(metadata1[0] == 12);
  REQUIRE(metadata1[1] == 44);
  auto metadata2 = def->metadata().get(attr2);
  REQUIRE(metadata2);
  REQUIRE(j::strings::string(metadata2) == "hello");
}

TEST_CASE("property metadata" * doctest::skip()) {
  auto xx_def = access::registry::get_typed_access_definition<xx>();
  REQUIRE(xx_def);
  REQUIRE(xx_def->is_object());
  auto & xxprop = xx_def->as_object().get_property("x");
  REQUIRE(xxprop.metadata().contains(attr1));
  REQUIRE(xxprop.metadata().get(attr1).size() == 1);
  REQUIRE(xxprop.metadata().get(attr1)[0] == 58);
  REQUIRE(!xxprop.metadata().contains(attr2));
  auto yy_def = access::registry::get_typed_access_definition<yy>();
  REQUIRE(yy_def);
  REQUIRE(yy_def->is_object());
  auto & yyprop = yy_def->as_object().get_property("x");
  REQUIRE(!yyprop.metadata().contains(attr1));
  REQUIRE(!yyprop.metadata().contains(attr2));
}

TEST_CASE("list metadata" * doctest::skip()) {
  auto list_def = access::registry::get_typed_access_definition<j::vector<u8_t>>();
  REQUIRE(list_def);
  REQUIRE(list_def->type == PROPERTY_TYPE::LIST);
  REQUIRE(list_def->is_list());
  const auto & md = list_def->metadata();
  REQUIRE(md.contains(attr1));
  REQUIRE(md.get(attr1).size() == 1);
  REQUIRE(md.get(attr1)[0] == -1);
  REQUIRE(!md.contains(attr2));
  const auto & imd = list_def->as_list().item_metadata();
  REQUIRE(imd.contains(attr1));
  REQUIRE(imd.get(attr1).size() == 2);
  REQUIRE(imd.get(attr1)[0] == 999);
  REQUIRE(imd.get(attr1)[1] == 1099);
  REQUIRE(imd.contains(attr2));
  REQUIRE(j::strings::string(imd.get(attr2)) == "listitem");
}

TEST_SUITE_END();
