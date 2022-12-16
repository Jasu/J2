#include <detail/preamble.hpp>

#include "bits/fixed_bitset.hpp"
#include "bits/fixed_bitset_refinement.hpp"

using namespace j::bits;


TEST_SUITE_BEGIN("Bits - fixed_bitset");

namespace {
  template<u32_t N> void check_basic(const fixed_bitset<N> & bs) {
    REQUIRE_UNARY(bs == bs);
    REQUIRE_EQ(bs.capacity(), N * 64U);
    REQUIRE_LE(bs.size(), bs.capacity());
    REQUIRE_UNARY((bs | bs) == bs);
    REQUIRE_UNARY((bs & bs) == bs);
    REQUIRE_UNARY(!(bs - bs));

    {
      u32_t num = 0U;
      for (u32_t i = 0U; i < N * 64U; ++i) {
        num += bs.has(i);
      }
      REQUIRE_EQ(bs.size(), num);
    }

    {
      u32_t num = 0U;
      i32_t prev_idx = -1;
      bool ok = true;
      for (u32_t idx : bs) {
        ok &= (i32_t)idx > prev_idx;
        ++num;
        prev_idx = idx;
      }
      REQUIRE_UNARY(ok);
      REQUIRE_LT(prev_idx, (i32_t)N * 64);
      REQUIRE_EQ(bs.size(), num);
    }

    {
      fixed_bitset<N> bs2{bs};
      REQUIRE_UNARY(bs == bs2);
    }

    {
      fixed_bitset<N> bs2;
      bs2 = bs;
      REQUIRE_UNARY(bs == bs2);
    }

    {
      fixed_bitset<N> bs2;
      REQUIRE_UNARY(bs == (bs | bs2));
      bs2 |= bs;
      REQUIRE_UNARY(bs == bs2);
    }
  }

  template<u32_t N> void check_empty_basic(const fixed_bitset<N> & bs) {
    check_basic(bs);
    REQUIRE_UNARY(!bs);
    REQUIRE_FALSE((bool)bs);
    REQUIRE_EQ(bs.size(), 0U);
    REQUIRE_UNARY(bs.begin().idx == N * 64U);
    REQUIRE_UNARY(bs.begin() == bs.end());
  }

  template<u32_t N> void check_not_empty_basic(const fixed_bitset<N> & bs, u32_t sz) {
    check_basic(bs);
    REQUIRE_FALSE(!bs);
    REQUIRE_UNARY((bool)bs);
    REQUIRE_EQ(bs.size(), sz);
    REQUIRE_UNARY(bs.begin() != bs.end());
  }

  template<u32_t N> void check_empty(const fixed_bitset<N> & bs) {
    check_empty_basic(bs);
    check_basic(bs & bs);
    check_basic(bs | bs);
    REQUIRE_UNARY((bs & bs) == bs);
    REQUIRE_UNARY((bs | bs) == bs);
    REQUIRE_UNARY(!(bs - bs));
    check_basic(bs - bs);
  }

  template<u32_t N> void check_not_empty(const fixed_bitset<N> & bs, u32_t sz) {
    check_not_empty_basic(bs, sz);
    REQUIRE_UNARY((bs & bs) == bs);
    REQUIRE_UNARY((bs | bs) == bs);
    REQUIRE_UNARY(!(bs - bs));
    check_basic(bs - bs);
  }
}

#define FBS bitset64, bitset128, fixed_bitset<3>, bitset256, fixed_bitset<5>, fixed_bitset<6>, fixed_bitset<7>, fixed_bitset<8>

TEST_CASE_TEMPLATE("Empty on init", T, FBS) {
  T t;
  check_empty(t);
}

TEST_CASE_TEMPLATE("Set unset single bit", T, FBS) {
  T t;
  for (u32_t i = 0U; i < t.capacity(); ++i) {
    t.add(i);
    check_not_empty(t, 1U);
    REQUIRE_UNARY(t.has(i));
    REQUIRE_UNARY(*t.begin() == i);
    REQUIRE_FALSE(t.has(i + 1U));
    if (i) {
      t.del(i - 1U);
      check_not_empty(t, 1U);
      REQUIRE_FALSE(t.has(i - 1U));
    }

    if (i < t.capacity() - 1) {
      t.del(i + 1U);
      check_not_empty(t, 1U);
      REQUIRE_FALSE(t.has(i + 1U));
    }
    REQUIRE_UNARY(t.has(i));

    t.del(i);
    check_empty(t);
  }
}

TEST_CASE_TEMPLATE("Set tail / head all ones", T, FBS) {
  T t;
  const u32_t cap = t.capacity();
  for (u32_t i = 0U; i < cap; ++i) {
    t.add(i);
    check_not_empty(t, i + 1U);
    bool ok = true;
    for (u32_t j = 0U; j < cap; ++j) {
      ok &= (j <= i) == t.has(j);
    }
    REQUIRE_UNARY(ok);
    u32_t cur = 0U;
    for (auto b : t) {
      ok &= cur == b;
      ++cur;
    }
    REQUIRE_UNARY(ok);
    REQUIRE_EQ(cur, i + 1U);
  }

  for (u32_t i = 0U; i < cap; ++i) {
    check_not_empty(t, cap - i);
    t.del(i);
    REQUIRE_EQ(t.size(), cap - i - 1U);
    bool ok = true;
    for (u32_t j = 0U; j < cap; ++j) {
      ok &= (j > i) == t.has(j);
    }
    u32_t cur = i + 1U;
    REQUIRE_UNARY(ok);
    for (auto b : t) {
      ok &= cur == b;
      ++cur;
    }
    REQUIRE_UNARY(ok);
    REQUIRE_EQ(cur, cap);
  }

  check_empty(t);
}

TEST_CASE_TEMPLATE("Set two bits", T, FBS) {
  const u32_t cap = T::capacity();
  bool ok = true;
  for (u32_t i = 0U; i < cap - 1U; ++i) {
    T a;
    a.add(i);
    for (u32_t j = i + 1U; j < cap; ++j) {
      T b;
      b.add(j);
      ok &= a != b;
      ok &= (a - b) == a;
      ok &= (b - a) == b;
      ok &= !(a & b);
      T c = a | b;
      ok &= c.size() == 2U;
      for (u32_t k = 0U; k < cap; ++k) {
        ok &= c.has(k) == (k == i || k == j);
      }
      auto it = c.begin();
      ok &= it != c.end();
      ok &= *it == i;
      ++it;
      ok &= it != c.end();
      ok &= *it == j;
      ++it;
      ok &= it == c.end();

      ok &= (c - a) == b;
      ok &= (c - b) == a;
      ok &= (c & a) == a;
      ok &= (c & b) == b;
      ok &= !(a - c);
      ok &= !(b - c);
    }
  }
  REQUIRE_UNARY(ok);
}

TEST_CASE_TEMPLATE("Fill", T, FBS) {
  const u32_t cap = T::capacity();
  T a;
  for (u32_t i = 0U; i <= cap; ++i) {
    a.fill(i);
    REQUIRE_EQ(a.size(), i);
    if (i) {
      REQUIRE_UNARY(a.has(0));
      REQUIRE_UNARY(a.has(i - 1));
    }
  }
  for (i32_t i = cap; i >= 0; --i) {
    a.fill(i);
    REQUIRE_EQ(a.size(), i);
    if (i) {
      REQUIRE_UNARY(a.has(0));
      REQUIRE_UNARY(a.has(i - 1));
    }
  }
}

TEST_CASE_TEMPLATE("Partition by threes", T, FBS) {
  fixed_bitset_refinement<T> ref;
  REQUIRE_UNARY(ref.size() == 0U);
  const u32_t cap = T::capacity();
  u32_t expected = 1U;
  u32_t max = 0U;
  for (u32_t i = 0U; i < cap - 3U; i += 2U) {
    T bs;
    bs.add(i);
    bs.add(i + 1);
    bs.add(i + 2);
    max = i + 3;
    ref.split(bs);
    REQUIRE_EQ(ref.size(), expected);
    auto refinement = ref.get_refinement(bs);
    REQUIRE_EQ(refinement.size(), i ? 2U : 1U);
    REQUIRE_UNARY(ref.get_set(refinement) == bs);
    expected += 2U;
  }

  T x;
  for (auto & r : ref) {
    x ^= r;
  }
  REQUIRE_EQ(x.size(), max);
}

TEST_SUITE_END();
