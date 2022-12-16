#include <detail/preamble.hpp>

#include "bits/bitset.hpp"

using j::bits::bitset;

TEST_SUITE_BEGIN("Bits - Bitset");

TEST_CASE("Initialized bitset is empty") {
  for (u32_t sz = 1; sz <= 153; ++sz) {
    bitset bs(sz);
    REQUIRE(bs.empty());
    REQUIRE(bs.capacity() == sz);
    for (u32_t i = 0; i < sz; ++i) {
      REQUIRE(!bs.is_set(i));
      REQUIRE(bs.find_first_set(i) == bitset::npos);
      REQUIRE(bs.find_and_clear_first_set(i) == bitset::npos);
      REQUIRE(bs.find_first_clear(i) == i);
    }
  }
}

TEST_CASE("Bitset copy works") {
  for (u32_t sz = 1; sz <= 153; ++sz) {
    bitset bs(sz);
    for (u32_t i = 0; i < sz; ++i) {
      bs.set(i);
    }
    bitset bs2(bs);
    for (u32_t i = 0; i < sz; ++i) {
      REQUIRE(bs2.is_set(i));
      REQUIRE(bs.is_set(i) == bs2.is_set(i));
    }

    bs.clear();
    REQUIRE(bs.empty());
    for (u32_t i = 0; i < sz; ++i) {
      if (i & 1) {
        bs.set(i);
      }
    }
    REQUIRE_UNARY(sz == 1 || !bs.empty());
    bitset bs3(bs);
    REQUIRE_UNARY(sz == 1 || !bs3.empty());
    for (u32_t i = 0; i < sz; ++i) {
      REQUIRE(bs3.is_set(i) == (i & 1));
      REQUIRE(bs.is_set(i) == bs3.is_set(i));
    }
  }
}

TEST_CASE("Find first set") {
  for (u32_t sz = 1; sz < 153; ++sz) {
    bitset bs(sz);
    for (u32_t i = 0; i < sz; ++i) {
      bs.set(i);
      REQUIRE(bs.is_set(i));
      bool did_find = false;
      for (auto b : bs) {
        REQUIRE(b == i);
        did_find = true;
      }
      REQUIRE(did_find);
      REQUIRE(bs.find_first_set(0) == i);
      REQUIRE(bs.find_first_set(i) == i);
      if (i != sz - 1) {
        REQUIRE(bs.find_first_set(i + 1) == bitset::npos);
      }
      REQUIRE(bs.find_and_clear_first_set(0) == i);
      REQUIRE(!bs.is_set(i));
    }

    if (sz > 1) {
      bs.set(0);
      bs.set(1);
      for (u32_t i = 1; i < sz - 1; ++i) {
        bs.set(i + 1);
        REQUIRE(bs.find_first_set(0) == i - 1);
        REQUIRE(bs.find_first_set(i) == i);
        REQUIRE(bs.find_first_set(i - 1) == i - 1);
        REQUIRE(bs.find_first_set(i + 1) == i + 1);
        REQUIRE(bs.find_and_clear_first_set(0) == i - 1);
        REQUIRE(!bs.is_set(i - 1));
      }
    }
  }
}

TEST_CASE("Find first clear") {
  bitset bs(153);
  for (int i = 0; i < 153; ++i) {
    REQUIRE(bs.find_first_clear(i) == i);
    REQUIRE(bs.find_and_set_first_clear(i) == i);
    REQUIRE(bs.find_first_clear(i) == ((i == 152) ? bitset::npos : i + 1));
    REQUIRE(bs.is_set(i));
  }

  bs = bitset(153);

  for (int i = 1; i < 153; ++i) {
    bs.set(i);
    REQUIRE(bs.find_first_clear(i - 1) == i - 1);
    REQUIRE(bs.find_first_clear(i) == ((i == 152) ? bitset::npos : i + 1));
    bs.reset(i);
  }
}

TEST_SUITE_END();
