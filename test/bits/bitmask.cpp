#include <detail/preamble.hpp>

#include "bits/bitmask.hpp"

using namespace j::bits;

namespace {
  enum class my_enum {
    first = 0,
    second,
    third,
    fourth,
  };

  enum class large_enum : sz_t {
    first = 0,
    second,
    third,
    fourth,

    bit63 = 63,
    bit64 = 64,
    bit127 = 127,
    bit128 = 128,
    bit191 = 191,
  };

  void require_empty(const bitmask<my_enum, my_enum::fourth> & bm) {
    REQUIRE(bm.empty());
    REQUIRE(bm.size() == 0);
    REQUIRE(bm.begin() == bm.end());
    REQUIRE(!bm.contains(my_enum::first));
    REQUIRE(!bm.contains(my_enum::second));
    REQUIRE(!bm.contains(my_enum::third));
    REQUIRE(!bm.contains(my_enum::fourth));
    REQUIRE(bm.find(my_enum::first) == bm.end());
    REQUIRE(bm.find(my_enum::second) == bm.end());
    REQUIRE(bm.find(my_enum::third) == bm.end());
    REQUIRE(bm.find(my_enum::fourth) == bm.end());
    REQUIRE(bm == bitmask<my_enum, my_enum::fourth>());
  }

  template<typename Bitmask, typename Value>
  void require_contents_impl(const Bitmask & bm, typename Bitmask::const_iterator & it, Value value) {
    REQUIRE(it != bm.end());
    REQUIRE(*it == value);
    REQUIRE(it == bm.find(value));
    ++it;
    REQUIRE(it != bm.begin());
    REQUIRE(bm.contains(value));
  }

  template<typename Bitmask, typename... Values>
  void require_contents(const Bitmask & bm, Values... values) {
    REQUIRE(!bm.empty());
    REQUIRE(bm.size() == sizeof...(Values));
    auto it = bm.begin();
    (require_contents_impl(bm, it, values), ...);
    REQUIRE(it == bm.end());
    REQUIRE(bm == Bitmask(values...));
    REQUIRE(!(bm != Bitmask(values...)));
  }

  template<typename Bitmask, typename Value>
  void test_successful_insert(Bitmask & bm, Value v) {
    auto p = bm.insert(v);
    REQUIRE(p.first != bm.end());
    REQUIRE(*p.first == v);
    REQUIRE(p.first == bm.find(v));
    REQUIRE(p.second);
  }

  template<typename Bitmask, typename Value>
  void test_unsuccessful_insert(Bitmask & bm, Value v) {
    auto p = bm.insert(v);
    REQUIRE(p.first != bm.end());
    REQUIRE(*p.first == v);
    REQUIRE(p.first == bm.find(v));
    REQUIRE(!p.second);
  }
}

TEST_CASE("bitmask - empty") {
  bitmask<my_enum, my_enum::fourth> bm;
  require_empty(bm);
}

namespace {
  void test_insert_one(my_enum value) {
    bitmask<my_enum, my_enum::fourth> bm;
    require_empty(bm);
    auto p = bm.insert(value);
    REQUIRE(*p.first == value);
    REQUIRE(p.second);
    require_contents(bm, value);
    auto p2 = bm.insert(value);
    REQUIRE(*p2.first == value);
    REQUIRE(!p2.second);
    REQUIRE(p.first == p2.first);
    require_contents(bm, value);
  }
}

TEST_CASE("bitmask - insert") {
  test_insert_one(my_enum::first);
  test_insert_one(my_enum::second);
  test_insert_one(my_enum::third);
  test_insert_one(my_enum::fourth);
}

TEST_CASE("bitmask - insert 2") {
    bitmask<my_enum, my_enum::fourth> bm;
    require_empty(bm);
    test_successful_insert(bm, my_enum::second);
    require_contents(bm, my_enum::second);
    test_successful_insert(bm, my_enum::fourth);
    require_contents(bm, my_enum::second, my_enum::fourth);
}

TEST_CASE("bitmask - construct with values") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::first);
  require_contents(bm, my_enum::first);

  bitmask<my_enum, my_enum::fourth> bm2(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm2, my_enum::second, my_enum::third, my_enum::fourth);
}

TEST_CASE("bitmask - copy construct") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm, my_enum::second, my_enum::third, my_enum::fourth);
  bitmask<my_enum, my_enum::fourth> bm2(bm);
  require_contents(bm2, my_enum::second, my_enum::third, my_enum::fourth);
}

TEST_CASE("bitmask - move construct") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm, my_enum::second, my_enum::third, my_enum::fourth);
  bitmask<my_enum, my_enum::fourth> bm2(static_cast<bitmask<my_enum, my_enum::fourth> &&>(bm));
  require_contents(bm2, my_enum::second, my_enum::third, my_enum::fourth);
}

TEST_CASE("bitmask - copy assign") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm, my_enum::second, my_enum::third, my_enum::fourth);
  bitmask<my_enum, my_enum::fourth> bm2;
  bm2 = bm;
  require_contents(bm2, my_enum::second, my_enum::third, my_enum::fourth);
}

TEST_CASE("bitmask - move assign") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm, my_enum::second, my_enum::third, my_enum::fourth);
  bitmask<my_enum, my_enum::fourth> bm2;
  bm2 = static_cast<bitmask<my_enum, my_enum::fourth> &&>(bm);
  require_contents(bm2, my_enum::second, my_enum::third, my_enum::fourth);
}

TEST_CASE("bitmask - erase by key") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  require_contents(bm, my_enum::second, my_enum::third, my_enum::fourth);
  REQUIRE(bm.erase(my_enum::second) == 1);
  require_contents(bm, my_enum::third, my_enum::fourth);
  REQUIRE(bm.erase(my_enum::second) == 0);
  require_contents(bm, my_enum::third, my_enum::fourth);
  REQUIRE(bm.erase(my_enum::third) == 1);
  require_contents(bm, my_enum::fourth);
  REQUIRE(bm.erase(my_enum::fourth) == 1);
  require_empty(bm);
}

TEST_CASE("bitmask - erase by iterator") {
  bitmask<my_enum, my_enum::fourth> bm(my_enum::second, my_enum::third, my_enum::fourth);
  auto it = bm.find(my_enum::third);
  it = bm.erase(it);
  require_contents(bm, my_enum::second, my_enum::fourth);
  REQUIRE(it != bm.end());
  REQUIRE(*it == my_enum::fourth);
  it = bm.erase(it);
  require_contents(bm, my_enum::second);
  REQUIRE(it == bm.end());
}

TEST_CASE("bitmask - sizes") {
  REQUIRE(sizeof(bitmask<my_enum, my_enum::fourth>) == 1);
  REQUIRE(sizeof(bitmask<int, 7>) == 1);
  REQUIRE(sizeof(bitmask<int, 8>) == 4);
  REQUIRE(sizeof(bitmask<int, 31>) == 4);
  REQUIRE(sizeof(bitmask<int, 32>) == 8);
  REQUIRE(sizeof(bitmask<int, 63>) == 8);
  REQUIRE(sizeof(bitmask<int, 64>) == 16);
  REQUIRE(sizeof(bitmask<int, 127>) == 16);
  REQUIRE(sizeof(bitmask<int, 128>) == 24);
}

TEST_CASE("bitmask - large insert") {
  bitmask<large_enum, large_enum::bit191> bm;
  REQUIRE(bm.empty());
  test_successful_insert(bm, large_enum::first);
  require_contents(bm, large_enum::first);
  test_unsuccessful_insert(bm, large_enum::first);
  test_successful_insert(bm, large_enum::bit63);
  require_contents(bm, large_enum::first, large_enum::bit63);
  test_unsuccessful_insert(bm, large_enum::bit63);
  test_successful_insert(bm, large_enum::bit64);
  require_contents(bm, large_enum::first, large_enum::bit63, large_enum::bit64);
  test_unsuccessful_insert(bm, large_enum::bit64);
  test_successful_insert(bm, large_enum::bit127);
  require_contents(bm, large_enum::first, large_enum::bit63, large_enum::bit64, large_enum::bit127);
  test_unsuccessful_insert(bm, large_enum::bit127);
  test_successful_insert(bm, large_enum::bit128);
  require_contents(bm, large_enum::first, large_enum::bit63, large_enum::bit64, large_enum::bit127, large_enum::bit128);
  test_unsuccessful_insert(bm, large_enum::bit128);
  test_successful_insert(bm, large_enum::bit191);
  require_contents(bm, large_enum::first, large_enum::bit63, large_enum::bit64, large_enum::bit127, large_enum::bit128, large_enum::bit191);
  test_unsuccessful_insert(bm, large_enum::bit191);
  require_contents(bm, large_enum::first, large_enum::bit63, large_enum::bit64, large_enum::bit127, large_enum::bit128, large_enum::bit191);
}

TEST_CASE("bitmask - large construct with values") {
  bitmask<large_enum, large_enum::bit191> bm(large_enum::first, large_enum::bit64, large_enum::bit191);
  require_contents(bm, large_enum::first, large_enum::bit64, large_enum::bit191);
}

TEST_CASE("bitmask - large erase by value") {
  bitmask<large_enum, large_enum::bit191> bm(large_enum::first, large_enum::bit64, large_enum::bit191);
  REQUIRE(bm.erase(large_enum::bit64) == 1);
  require_contents(bm, large_enum::first, large_enum::bit191);
  REQUIRE(bm.erase(large_enum::bit64) == 0);
  require_contents(bm, large_enum::first, large_enum::bit191);
  REQUIRE(bm.erase(large_enum::bit191) == 1);
  require_contents(bm, large_enum::first);
  REQUIRE(bm.erase(large_enum::bit191) == 0);
  require_contents(bm, large_enum::first);
  REQUIRE(bm.erase(large_enum::first) == 1);
  REQUIRE(bm.empty());
  REQUIRE(bm.size() == 0);
  REQUIRE(bm.erase(large_enum::first) == 0);
  REQUIRE(bm.empty());
  REQUIRE(bm.size() == 0);
}

TEST_CASE("bitmask - large erase by iterator") {
  bitmask<large_enum, large_enum::bit191> bm(large_enum::first, large_enum::bit63, large_enum::bit64, large_enum::bit191);
  auto it = bm.find(large_enum::bit63);
  REQUIRE(it != bm.end());
  REQUIRE(*it == large_enum::bit63);
  it = bm.erase(it);
  require_contents(bm, large_enum::first, large_enum::bit64, large_enum::bit191);
  REQUIRE(it != bm.end());
  REQUIRE(*it == large_enum::bit64);
  it = bm.erase(it);
  require_contents(bm, large_enum::first, large_enum::bit191);
  REQUIRE(it != bm.end());
  REQUIRE(*it == large_enum::bit191);
  it = bm.erase(it);
  require_contents(bm, large_enum::first);
  REQUIRE(it == bm.end());
}

TEST_CASE("bitmask - operator|=") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bm1 |= bm2;
  require_contents(bm1, large_enum::first, large_enum::bit63, large_enum::bit191);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
}

TEST_CASE("bitmask - operator&=") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bm1 &= bm2;
  require_contents(bm1, large_enum::bit63);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
}

TEST_CASE("bitmask - operator^=") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bm1 ^= bm2;
  require_contents(bm1, large_enum::first, large_enum::bit191);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
}

TEST_CASE("bitmask - operator|") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bitmask<large_enum, large_enum::bit191> bm3 = bm1 | bm2;
  require_contents(bm1, large_enum::first, large_enum::bit63);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
  require_contents(bm3, large_enum::first, large_enum::bit63, large_enum::bit191);
}

TEST_CASE("bitmask - operator&") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bitmask<large_enum, large_enum::bit191> bm3 = bm1 & bm2;
  require_contents(bm1, large_enum::first, large_enum::bit63);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
  require_contents(bm3, large_enum::bit63);
}

TEST_CASE("bitmask - operator^") {
  bitmask<large_enum, large_enum::bit191> bm1(large_enum::first, large_enum::bit63);
  bitmask<large_enum, large_enum::bit191> bm2(large_enum::bit63, large_enum::bit191);
  bitmask<large_enum, large_enum::bit191> bm3 = bm1 ^ bm2;
  require_contents(bm1, large_enum::first, large_enum::bit63);
  require_contents(bm2, large_enum::bit63, large_enum::bit191);
  require_contents(bm3, large_enum::first, large_enum::bit191);
}
