#include <detail/preamble.hpp>

#include "bits/bitops.hpp"

TEST_SUITE_BEGIN("Bits - Bitops");

using namespace j::bits;

namespace {
  template<typename T>
  void test_popcount() {
    T v = 0;
    REQUIRE(popcount(v) == 0);
    v = 1;
    REQUIRE(popcount(v) == 1);
    v = -1;
    REQUIRE(popcount(v) == sizeof(T) * 8);
    v = 1UL << (sizeof(T) * 8 - 1);
    REQUIRE(popcount(v) == 1);
  }
}

TEST_CASE("popcount") {
  test_popcount<u8_t>();
  test_popcount<u16_t>();
  test_popcount<u32_t>();
  test_popcount<u64_t>();
}

namespace {
  template<typename T>
  void test_ctz() {
    T v = 1;
    REQUIRE(ctz(v) == 0);
    REQUIRE(ctz_safe(v) == 0);
    v = 2;
    REQUIRE(ctz(v) == 1);
    REQUIRE(ctz_safe(v) == 1);
    v = -1;
    REQUIRE(ctz(v) == 0);
    REQUIRE(ctz_safe(v) == 0);
    v = 1UL << (sizeof(T) * 8 - 1);
    REQUIRE(ctz(v) == sizeof(T) * 8 - 1);
    REQUIRE(ctz_safe(v) == sizeof(T) * 8 - 1);
    v = 0;
    REQUIRE(ctz_safe(v) == sizeof(T) * 8);
  }
}

TEST_CASE("ctz") {
  test_ctz<u8_t>();
  test_ctz<u16_t>();
  test_ctz<u32_t>();
  test_ctz<u64_t>();
}

namespace {
  template<typename T>
  void test_clz() {
    T v = 1;
    REQUIRE(clz(v) == sizeof(T) * 8 - 1);
    REQUIRE(clz_safe(v) == sizeof(T) * 8 - 1);
    v = 2;
    REQUIRE(clz(v) == sizeof(T) * 8 - 2);
    REQUIRE(clz_safe(v) == sizeof(T) * 8 - 2);
    v = -1;
    REQUIRE(clz(v) == 0);
    REQUIRE(clz_safe(v) == 0);
    v = 0;
    REQUIRE(clz_safe(v) == sizeof(T) * 8);
  }
}

TEST_CASE("clz") {
  test_clz<u8_t>();
  test_clz<u16_t>();
  test_clz<u32_t>();
  test_clz<u64_t>();
}

TEST_CASE("mask") {
  REQUIRE(mask<u64_t>(0) == 0);
  REQUIRE(mask<u64_t>(1) == 1);
  REQUIRE(mask<u64_t>(2) == 3);
  REQUIRE(mask<u64_t>(3) == 7);
  REQUIRE(mask<u64_t>(4) == 15);
  REQUIRE(mask<u64_t>(64) == 0xFFFFFFFFFFFFFFFFULL);
}

TEST_CASE("mask with offset") {
  REQUIRE(mask<u64_t>(0, 0) == 0);
  REQUIRE(mask<u64_t>(0, 1) == 0);
  REQUIRE(mask<u64_t>(1, 0) == 1);
  REQUIRE(mask<u64_t>(1, 1) == 2);
  REQUIRE(mask<u64_t>(2, 0) == 3);
  REQUIRE(mask<u64_t>(2, 1) == 6);
  REQUIRE(mask<u64_t>(3, 0) == 7);
  REQUIRE(mask<u64_t>(3, 1) == 14);
  REQUIRE(mask<u64_t>(4, 0) == 15);
  REQUIRE(mask<u64_t>(4, 1) == 30);

  REQUIRE(mask<u64_t>(1, 63) == (1UL << 63));
}

TEST_CASE("bit_width") {
  REQUIRE(bit_width(1) == 1);
  REQUIRE(bit_width(2) == 2);
  REQUIRE(bit_width(3) == 2);
  REQUIRE(bit_width(4) == 3);
  REQUIRE(bit_width(5) == 3);
  REQUIRE(bit_width(6) == 3);
  REQUIRE(bit_width(7) == 3);

  REQUIRE(bit_width(1UL << 63) == 64);
  REQUIRE(bit_width((sz_t)-1LL) == 64);
}

TEST_SUITE_END();
