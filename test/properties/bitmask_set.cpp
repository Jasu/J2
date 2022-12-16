#include <detail/preamble.hpp>

#include "properties/set_registration.hpp"
#include "properties/enum_registration.hpp"
#include "bits/bitmask.hpp"

using namespace j::properties;
using namespace j::bits;

namespace {
  enum class my_enum {
    first = 0,
    second,
    third,
  };
  using bitmask_t = bitmask<my_enum, my_enum::third>;

  enum_access_registration<my_enum> my_enum_reg{
    "my_enum",
    value = enum_value(my_enum::first, "first"),
    value = enum_value(my_enum::second, "second"),
    value = enum_value(my_enum::third, "third"),
  };

  static set_access_registration<bitmask_t> bitmask_reg("bitmask<my_enum>");

  void require_set(set_access & a) {
    REQUIRE(a.type() == PROPERTY_TYPE::SET);
    REQUIRE(a.is_set());
    REQUIRE(&a.as_set() == &a);
  }

  void require_empty(set_access & a) {
    REQUIRE(a.empty());
    REQUIRE(a.size() == 0);
    REQUIRE(a.begin() == a.end());
    REQUIRE(a.find(my_enum::first) == a.end());
    REQUIRE(!a.contains(my_enum::first));
    REQUIRE(a.find(my_enum::second) == a.end());
    REQUIRE(!a.contains(my_enum::second));
    REQUIRE(a.find(my_enum::third) == a.end());
    REQUIRE(!a.contains(my_enum::third));
  }

  template<typename Value>
  void require_contents_impl(set_access & a, Value v) {
    REQUIRE(a.contains(v));
    REQUIRE(a.find(v) != a.end());
    REQUIRE((*a.find(v)).is_enum());
    REQUIRE((*a.find(v)).as_enum().template value<my_enum>() == v);
  }

  template<typename... Values>
  void require_contents(set_access & a, Values... vs) {
    REQUIRE(!a.empty());
    REQUIRE(a.size() == sizeof...(Values));
    (require_contents_impl(a, vs), ...);
  }

  void test_successful_insert(set_access & a, my_enum v) {
    auto p = a.insert(v);
    REQUIRE(p.second);
    REQUIRE(p.first != a.end());
    REQUIRE((*p.first).is_enum());
    REQUIRE((*p.first).as_enum().value<my_enum>() == v);
  }

  void test_unsuccessful_insert(set_access & a, my_enum v) {
    auto p = a.insert(v);
    REQUIRE(!p.second);
    REQUIRE(p.first != a.end());
    REQUIRE((*p.first).is_enum());
    REQUIRE((*p.first).as_enum().value<my_enum>() == v);
  }
}

TEST_CASE("empty bitmask access") {
  bitmask_t set;
  set_access a(set);
  require_set(a);
  require_empty(a);
}

TEST_CASE("one element bitmask") {
  bitmask_t set(my_enum::first);
  set_access a(set);
  require_set(a);
  require_contents(a, my_enum::first);
}

TEST_CASE("bitmask insert") {
  bitmask_t set;
  set_access a(set);
  require_set(a);
  require_empty(a);
  test_successful_insert(a, my_enum::first);
  require_contents(a, my_enum::first);
  test_unsuccessful_insert(a, my_enum::first);
  require_contents(a, my_enum::first);
  test_successful_insert(a, my_enum::second);
  require_contents(a, my_enum::first, my_enum::second);
}

TEST_CASE("bitmask erase by item") {
  bitmask_t set(my_enum::first, my_enum::second, my_enum::third);
  set_access a(set);
  require_set(a);
  require_contents(a, my_enum::first, my_enum::second, my_enum::third);
  REQUIRE(a.erase(my_enum::third) == 1);
  require_contents(a, my_enum::first, my_enum::second);
  REQUIRE(a.erase(my_enum::third) == 0);
  require_contents(a, my_enum::first, my_enum::second);
  REQUIRE(a.erase(my_enum::first) == 1);
  require_contents(a, my_enum::second);
  REQUIRE(a.erase(my_enum::second) == 1);
  require_empty(a);
}

TEST_CASE("bitmask erase by iterator") {
  bitmask_t set(my_enum::first, my_enum::second, my_enum::third);
  set_access a(set);
  require_set(a);
  require_contents(a, my_enum::first, my_enum::second, my_enum::third);
  auto it = a.find(my_enum::first);
  a.erase(it);
  require_contents(a, my_enum::second, my_enum::third);
}

TEST_CASE("bitmask erase by iterator fully") {
  bitmask_t set(my_enum::first, my_enum::second, my_enum::third);
  set_access a(set);
  require_set(a);
  require_contents(a, my_enum::first, my_enum::second, my_enum::third);
  sz_t i = 0;
  for (auto it = a.begin(); it != a.end(); it = a.erase(it)) {
    ++i;
  }
  require_empty(a);
  REQUIRE(i == 3);
}
