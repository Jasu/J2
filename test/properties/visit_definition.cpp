#include <detail/preamble.hpp>

#include "properties/visiting/visit_definition_recursive.hpp"
#include "properties/access/default_accesses.hpp"
#include "properties/access/registry.hpp"
#include "properties/access/list_access_registration.hpp"
#include "properties/access/map_access_registration.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "properties/classes/object_access_registration.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"
#include "hzd/types.hpp"

using namespace j::properties;
using namespace j::properties::visiting;
using namespace j::properties::access;
using namespace j::properties::classes;

namespace {
  void assert_contains_access_definition(
    j::span<const typed_access_definition* const> defs,
    const typed_access_definition* def)
  {
    REQUIRE(def);
    bool contains = false;
    for (auto d : defs) {
      if (d == def) {
        REQUIRE(!contains);
        contains = true;
      }
    }
    INFO("Expected definition: " << def->get_name());
    REQUIRE(contains);
  }

  template<typename... AccessDefinitions>
  void assert_access_definitions(const visit_definition_path & path, AccessDefinitions ... definitions) {
    REQUIRE(path.access_definitions().size() == sizeof...(definitions));
    (assert_contains_access_definition(path.access_definitions(), definitions), ...);
  }

  template<typename Target, typename... AccessDefinitions>
  void expect_single_visit(Target && target, AccessDefinitions ... definitions) {
    sz_t count = 0;

    visit_definition_recursive(
      target,
      [&count, definitions...](const visit_definition_path & path) -> void {
        ++count;
        REQUIRE(!path.property());
        REQUIRE(path.type() == DEFINITION_COMPONENT_TYPE::ROOT);
        REQUIRE(path.is_root());
        REQUIRE(path.parent() == nullptr);
        REQUIRE(path.access_definitions().size() == sizeof...(definitions));
        (assert_contains_access_definition(path.access_definitions(), definitions), ...);
      });
    REQUIRE(count == 1);
  }
}

TEST_CASE("Nil definition visit") {
  expect_single_visit(*typed_access_definition_v<null_t>, typed_access_definition_v<null_t>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<null_t>(),
    typed_access_definition_v<null_t>);
}

TEST_CASE("Bool definition visit") {
  expect_single_visit(*typed_access_definition_v<bool>, typed_access_definition_v<bool>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<bool>(),
    typed_access_definition_v<bool>);
}

TEST_CASE("Int definition visit") {
  expect_single_visit(*typed_access_definition_v<i8_t>, typed_access_definition_v<i8_t>);
  expect_single_visit(*typed_access_definition_v<i16_t>, typed_access_definition_v<i16_t>);
  expect_single_visit(*typed_access_definition_v<i32_t>, typed_access_definition_v<i32_t>);
  expect_single_visit(*typed_access_definition_v<i64_t>, typed_access_definition_v<i64_t>);
  expect_single_visit(*typed_access_definition_v<u8_t>, typed_access_definition_v<u8_t>);
  expect_single_visit(*typed_access_definition_v<u16_t>, typed_access_definition_v<u16_t>);
  expect_single_visit(*typed_access_definition_v<u32_t>, typed_access_definition_v<u32_t>);
  expect_single_visit(*typed_access_definition_v<u64_t>, typed_access_definition_v<u64_t>);

  expect_single_visit(*access::registry::get_wrapper_definition<i8_t>(),
    typed_access_definition_v<i8_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<i16_t>(),
    typed_access_definition_v<i16_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<i32_t>(),
    typed_access_definition_v<i32_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<i64_t>(),
    typed_access_definition_v<i64_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<u8_t>(),
    typed_access_definition_v<u8_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<u16_t>(),
    typed_access_definition_v<u16_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<u32_t>(),
    typed_access_definition_v<u32_t>);
  expect_single_visit(*access::registry::get_wrapper_definition<u64_t>(),
    typed_access_definition_v<u64_t>);
}

TEST_CASE("Float definition visit") {
  expect_single_visit(*typed_access_definition_v<float>, typed_access_definition_v<float>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<float>(),
    typed_access_definition_v<float>);
}

TEST_CASE("Double definition visit") {
  expect_single_visit(*typed_access_definition_v<double>, typed_access_definition_v<double>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<double>(),
    typed_access_definition_v<double>);
}

TEST_CASE("String definition visit") {
  expect_single_visit(*typed_access_definition_v<j::strings::string>, typed_access_definition_v<j::strings::string>);
  expect_single_visit(*typed_access_definition_v<const char*>, typed_access_definition_v<const char*>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<j::strings::string>(),
    typed_access_definition_v<j::strings::string>);

  expect_single_visit(
    *access::registry::get_wrapper_definition<const char *>(),
    typed_access_definition_v<const char*>);
}


using i8_t_list_reg_t J_NO_DEBUG_TYPE = list_access_registration<j::vector<i8_t>>;
static i8_t_list_reg_t i8_t_list_reg("vector<i8_t>");
TEST_CASE("List definition visit") {
  bool has_root = false, has_item = false;
  visit_definition_recursive(
    i8_t_list_reg_t::definition,
    [&has_root, &has_item](const visit_definition_path & path) -> void {
      REQUIRE(!path.property());
      if (path.is_root()) {
        REQUIRE(!has_root);
        REQUIRE(!path.parent());
        has_root = true;
        assert_access_definitions(path, &i8_t_list_reg_t::definition);
      } else if (path.is_list_item()) {
        REQUIRE(has_root);
        REQUIRE(!has_item);
        has_item = true;
        REQUIRE(path.parent());
        REQUIRE(path.parent()->is_root());
        REQUIRE(!path.parent()->parent());
        assert_access_definitions(*path.parent(), &i8_t_list_reg_t::definition);
        assert_access_definitions(path, typed_access_definition_v<i8_t>);
      } else {
        FAIL("Unexpected path component type.");
      }
    });
  REQUIRE(has_root);
  REQUIRE(has_item);
}

struct J_TYPE_HIDDEN object_pair {
  int x;
  bool y;
};

using object_pair_reg_t J_NO_DEBUG_TYPE = object_access_registration<object_pair>;
static object_pair_reg_t object_pair_reg{
  "object_pair",
  property = member<&object_pair::x>("x"),
  property = member<&object_pair::y>("y"),
};

TEST_CASE("Object definition visit") {
  bool has_root = false, has_x = false, has_y = false;
  visit_definition_recursive(
    object_pair_reg_t::definition,
    [&has_root, &has_x, &has_y](const visit_definition_path & path) -> void {
      if (path.is_root()) {
        REQUIRE(!has_root);
        REQUIRE(!path.parent());
        REQUIRE(!path.property());
        has_root = true;
        assert_access_definitions(path, &object_pair_reg_t::definition);
      } else if (path.is_object_property()) {
        REQUIRE(has_root);
        REQUIRE(path.property());
        REQUIRE(path.parent());
        REQUIRE(path.parent()->is_root());
        REQUIRE(!path.parent()->parent());
        assert_access_definitions(*path.parent(), &object_pair_reg_t::definition);
        if (path.property()->name() == "x") {
          REQUIRE(!has_x);
          has_x = true;
          assert_access_definitions(path, typed_access_definition_v<i32_t>);
        } else if (path.property()->name() == "y") {
          REQUIRE(!has_y);
          has_y = true;
          assert_access_definitions(path, typed_access_definition_v<bool>);
        } else {
          FAIL("Unexpected property name.");
        }
      } else {
        FAIL("Unexpected path type.");
      }
    });
  REQUIRE(has_root);
  REQUIRE(has_x);
  REQUIRE(has_y);
}
