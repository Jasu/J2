#include <detail/preamble.hpp>

#include "properties/set_registration.hpp"
#include "properties/access/int_access.hpp"

#include <set>

using namespace j::properties;

namespace {
  using set_t J_NO_DEBUG_TYPE = std::set<u16_t>;
  static set_access_registration<set_t> set_reg("set<u16_t>");
  void require_set(set_access & a) {
    REQUIRE(a.type() == PROPERTY_TYPE::SET);
    REQUIRE(a.is_set());
    REQUIRE(&a.as_set() == &a);
  }

  void require_empty(set_access & a) {
    REQUIRE(a.empty());
    REQUIRE(a.size() == 0);
    REQUIRE(a.begin() == a.end());
    REQUIRE(a.find(0) == a.end());
    REQUIRE(!a.contains(0));
  }

  template<typename Value>
  void require_contents_impl(set_access & a, Value v) {
    REQUIRE(a.contains(v));
    REQUIRE(a.find(v) != a.end());
    REQUIRE((*a.find(v)).is_int());
    REQUIRE((*a.find(v)).as_int().get_value() == v);
  }

  template<typename... Values>
  void require_contents(set_access & a, Values... vs) {
    REQUIRE(!a.empty());
    REQUIRE(a.size() == sizeof...(Values));
    (require_contents_impl(a, vs), ...);
  }

  template<typename Value>
  void test_successful_insert(set_access & a, Value v) {
    auto p = a.insert(v);
    REQUIRE(p.second);
    REQUIRE(p.first != a.end());
    REQUIRE((*p.first).is_int());
    REQUIRE((*p.first).as_int().get_value() == v);
  }

  template<typename Value>
  void test_unsuccessful_insert(set_access & a, Value v) {
    auto p = a.insert(v);
    REQUIRE(!p.second);
    REQUIRE(p.first != a.end());
    REQUIRE((*p.first).is_int());
    REQUIRE((*p.first).as_int().get_value() == v);
  }
}

TEST_CASE("empty set access") {
  set_t set;
  set_access a(set);
  require_set(a);
  require_empty(a);
}

TEST_CASE("one element set") {
  set_t set;
  set.emplace(123);
  set_access a(set);
  require_set(a);
  require_contents(a, 123);
}

TEST_CASE("set insert") {
  set_t set;
  set_access a(set);
  require_set(a);
  require_empty(a);
  test_successful_insert(a, 123);
  require_contents(a, 123);
  test_unsuccessful_insert(a, 123);
  require_contents(a, 123);
  test_successful_insert(a, 124);
  require_contents(a, 123, 124);
}

TEST_CASE("set erase by item") {
  set_t set;
  set.emplace(123);
  set.emplace(999);
  set.emplace(111);
  set_access a(set);
  require_set(a);
  require_contents(a, 123, 111, 999);
  REQUIRE(a.erase(123) == 1);
  require_contents(a, 111, 999);
  REQUIRE(a.erase(123) == 0);
  require_contents(a, 111, 999);
  REQUIRE(a.erase(111) == 1);
  require_contents(a, 999);
  REQUIRE(a.erase(999) == 1);
  require_empty(a);
}

TEST_CASE("set erase by iterator") {
  set_t set({ 123, 999, 111 });
  set_access a(set);
  require_set(a);
  require_contents(a, 123, 111, 999);
  auto it = a.find(111);
  a.erase(it);
  require_contents(a, 123, 999);
}

TEST_CASE("set erase by iterator fully") {
  set_t set({ 123, 999, 111 });
  set_access a(set);
  require_set(a);
  require_contents(a, 123, 111, 999);
  sz_t i = 0;
  for (auto it = a.begin(); it != a.end(); it = a.erase(it)) {
    ++i;
  }
  require_empty(a);
  REQUIRE(i == 3);
}
