#include <detail/preamble.hpp>

#include "math/interval.hpp"

#define TEST_CASE_ALL(N) \
  TEST_CASE_TEMPLATE(N, Iv, iv<u8_t>, iv<u16_t>, iv<u32_t>, iv<u64_t>, iv<i8_t>, iv<i16_t>, iv<i32_t>, iv<i64_t>)

TEST_SUITE_BEGIN("Math - Interval");
namespace {
  namespace m = j::math;
  template<typename T> using iv = m::interval<T>;

  template<typename A>
  inline void validate_interval(const iv<A> & a) {
    REQUIRE_UNARY(a == a);
    REQUIRE(!a == (a.size() == 0));
    REQUIRE(j::add_overflow(a.left(), a.size()) == a.right());
    REQUIRE_UNARY(a.contains(a));
    REQUIRE((bool)a == a.contains(a.left()));
    REQUIRE((bool)a == a.contains(j::add_overflow(a.left(), a.size() - 1U)));
    REQUIRE((bool)a == a.intersects(a));
    REQUIRE_UNARY(a.intersection(a) == a);
    REQUIRE(a.is_overflown() == (a.left() > a.right()));
  }

  template<typename A>
  inline void assert_interval(const iv<A> & a, j::type_identity_t<A> left, j::type_identity_t<A> right) {
    validate_interval(a);
    REQUIRE(a.left() == left);
    REQUIRE(a.right() == right);
  }
}

TEST_CASE_ALL("Empty") {
  Iv i;
  SUBCASE("Default")      { }
  SUBCASE("By endpoints") { i = Iv{m::by_endpoints, 0, 0}; }
  SUBCASE("By size")      { i = Iv{m::by_size, 0, 0}; }
  assert_interval(i, 0, 0);
}

TEST_CASE_ALL("One") {
  Iv i;
  SUBCASE("By endpoints") { i = Iv{m::by_endpoints, 1, 2}; }
  SUBCASE("By size")      { i = Iv{m::by_size, 1, 1}; }
  assert_interval(i, 1, 2);
}

TEST_CASE_ALL("Max fixed") {
  Iv i;
  SUBCASE("By endpoints") { i = Iv{m::by_endpoints, Iv::min_v, Iv::max_v}; }
  SUBCASE("By size")      { i = Iv{m::by_size, Iv::min_v, Iv::max_size_v}; }
  assert_interval(i, Iv::min_v, Iv::max_v);
}

TEST_SUITE_END();
