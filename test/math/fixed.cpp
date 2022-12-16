#include <detail/preamble.hpp>

#include "math/fixed.hpp"

using namespace j::math;

namespace {
  template<typename A, typename B>
  J_NO_INLINE void require_equals(const A & a, const B & b) {
    REQUIRE_UNARY(a == b);
    REQUIRE_UNARY_FALSE(a != b);
    REQUIRE_UNARY(a <= b);
    REQUIRE_UNARY(a >= b);
    REQUIRE_UNARY_FALSE(a < b);
    REQUIRE_UNARY_FALSE(a > b);
  }

  template<typename Fn>
  void with_all(Fn && fn) {
    fn(s16_16{});
    fn(s24_8{});
    fn(s8_24{});
    fn(u16_16{});
    fn(u24_8{});
    fn(u8_24{});
  }

  template<typename A, typename B>
  [[nodiscard]] constexpr A common_highest() noexcept {
    if constexpr (highest_v<A> < highest_v<B>) {
      return A(B(highest_v<A>));
    } else {
      return A(highest_v<B>);
    }
  }

  template<typename A, typename B>
  [[nodiscard]] constexpr A common_lowest() noexcept {
    if constexpr (lowest_v<A> < lowest_v<B>) {
      return A(lowest_v<B>);
    } else {
      return A(B(lowest_v<A>));
    }
  }

  template<typename A, typename B>
  [[nodiscard]] constexpr A common_highest_int() noexcept {
    if constexpr (u64_t(A::highest_int) > u64_t(B::highest_int)) {
      return B::highest_int;
    } else {
      return A::highest_int;
    }
  }

  template<typename A, typename B>
  [[nodiscard]] constexpr A common_lowest_int() noexcept {
    if constexpr (i64_t(A::lowest_int) > i64_t(B::lowest_int)) {
      return A::lowest_int;
    } else {
      return B::lowest_int;
    }
  }
}

TEST_CASE("fixed-point large value comparisons") {
  REQUIRE_UNARY(highest_v<s24_8> > highest_v<s16_16>);
  REQUIRE_UNARY(highest_v<s24_8> > highest_v<s8_24>);
  REQUIRE_UNARY(highest_v<s24_8> > highest_v<u16_16>);
  REQUIRE_UNARY(highest_v<s24_8> > highest_v<u8_24>);

  REQUIRE_UNARY(highest_v<s16_16> > highest_v<s8_24>);
  REQUIRE_UNARY(highest_v<s16_16> > highest_v<u8_24>);

  REQUIRE_UNARY(highest_v<u24_8> > highest_v<u16_16>);
  REQUIRE_UNARY(highest_v<u24_8> > highest_v<u8_24>);
  REQUIRE_UNARY(highest_v<u24_8> > highest_v<s24_8>);
  REQUIRE_UNARY(highest_v<u24_8> > highest_v<s16_16>);
  REQUIRE_UNARY(highest_v<u24_8> > highest_v<s8_24>);

  REQUIRE_UNARY(highest_v<u16_16> > highest_v<s16_16>);
  REQUIRE_UNARY(highest_v<u16_16> > highest_v<s8_24>);
  REQUIRE_UNARY(highest_v<u16_16> > highest_v<u8_24>);

  REQUIRE_UNARY(highest_v<u8_24> > highest_v<s8_24>);
}

TEST_CASE("fixed-point small value comparisons") {
  REQUIRE_UNARY(lowest_v<s24_8> < lowest_v<s16_16>);
  REQUIRE_UNARY(lowest_v<s24_8> < lowest_v<s8_24>);
  REQUIRE_UNARY(lowest_v<s24_8> < lowest_v<u8_24>);
  REQUIRE_UNARY(lowest_v<s24_8> < lowest_v<u16_16>);
  REQUIRE_UNARY(lowest_v<s24_8> < lowest_v<u24_8>);

  REQUIRE_UNARY(lowest_v<s16_16> < lowest_v<s8_24>);
  REQUIRE_UNARY(lowest_v<s16_16> < lowest_v<u8_24>);
  REQUIRE_UNARY(lowest_v<s16_16> < lowest_v<u16_16>);
  REQUIRE_UNARY(lowest_v<s16_16> < lowest_v<u24_8>);

  REQUIRE_UNARY(lowest_v<s8_24> < lowest_v<u8_24>);
  REQUIRE_UNARY(lowest_v<s8_24> < lowest_v<u16_16>);
  REQUIRE_UNARY(lowest_v<s8_24> < lowest_v<u24_8>);

  REQUIRE_UNARY(lowest_v<u8_24> == lowest_v<u16_16>);
  REQUIRE_UNARY(lowest_v<u8_24> == lowest_v<u24_8>);
  REQUIRE_UNARY(lowest_v<u16_16> == lowest_v<u24_8>);
}

TEST_CASE_TEMPLATE("fixed-point comparison", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  with_all([](auto b) {
    require_equals(TestType{0}, decltype(b){0});
    require_equals(TestType{1}, decltype(b){1});
    require_equals(common_highest<TestType, decltype(b)>(), common_highest<decltype(b), TestType>());
    require_equals(common_highest_int<TestType, decltype(b)>(), common_highest_int<decltype(b), TestType>());
    require_equals(common_lowest<TestType, decltype(b)>(), common_lowest<decltype(b), TestType>());
    require_equals(common_lowest_int<TestType, decltype(b)>(), common_lowest_int<decltype(b), TestType>());
  });
}

namespace {
  template<typename A, typename B, typename C>
  void test_multiplication(A a, const B & b, const C & c) {
    REQUIRE_UNARY(a * b == c);
    REQUIRE_UNARY(b * a == c);
    a *= b;
    REQUIRE_UNARY(a == c);
  }

  template<typename T>
  struct J_TYPE_HIDDEN value_tuple {
    T a;
    int b;
    int c;
  };
}

TEST_CASE_TEMPLATE("fixed-point multiply by int", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  const value_tuple<TestType> multiplication_values[]{
    {123, 1, 123},
    {34, 2, 68},
    {127, 0, 0},
    {0, 127, 0},
  };
  for (int i = 0; i < 4; ++i) {
    test_multiplication(multiplication_values[i].a,
                        multiplication_values[i].b,
                        multiplication_values[i].c);
  }

  if constexpr (TestType::is_signed) {
    const value_tuple<TestType> multiplication_values_neg[]{
      {-1, 1, -1},
      {1, -1, -1},
      {-1, -1, 1},
      {-11, 0, 0},
      {0, -11, 0},
      {-11, 10, -110},
      {11, -10, -110},
      {-11, -10, 110},
    };
  for (int i = 0; i < 8; ++i) {
    test_multiplication(multiplication_values_neg[i].a,
                        multiplication_values_neg[i].b,
                        multiplication_values_neg[i].c);
  }
  }
}

TEST_CASE_TEMPLATE("fixed-point multiply by float", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  test_multiplication(123, 1.0, 123);
  test_multiplication(34, 2.0, 68);
  test_multiplication(127, 0.0, 0);
  test_multiplication(0, 127.0, 0);
  test_multiplication(50, 0.5, 25);

  if constexpr (TestType::is_signed) {
    test_multiplication(-1, 1.0, -1);
    test_multiplication(1, -1.0, -1);
    test_multiplication(-1, -1.0, 1);
    test_multiplication(-11, 0.0, 0);
    test_multiplication(0, -11.0, 0);
    test_multiplication(-11, 10.0, -110);
    test_multiplication(11, -10.0, -110);
    test_multiplication(-11, -10.0, 110);
    test_multiplication(-20, 0.5, -10);
    test_multiplication(20, -0.5, -10);
    test_multiplication(-20, -0.5, 10);
  }
}

namespace {
  template<typename A, typename B, typename C>
  void test_division(A a, const B & b, const C & c) {
    REQUIRE_UNARY(a / b == c);
    a /= b;
    REQUIRE_UNARY(a == c);
  }
}

TEST_CASE_TEMPLATE("fixed-point divide by int", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  test_division(123, 1, 123);
  test_division(34, 2, 17);
  test_division(127, 127, 1);
  test_division(0, 1, 0);

  if constexpr (TestType::is_signed) {
    test_division(-1, 1, -1);
    test_division(1, -1, -1);
    test_division(-1, -1, 1);
    test_division(0, -1, 0);
    test_division(-110, 10, -11);
    test_division(110, -10, -11);
    test_division(-110, -10, 11);
  }
}

TEST_CASE_TEMPLATE("fixed-point divide by float", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  test_division(123, 1.0, 123);
  test_division(34, 2.0, 17);
  test_division(34, 0.5, 68);
  test_division(127, 127.0, 1);
  test_division(0, 1.0, 0);
  test_division(0, 0.5, 0);

  if constexpr (TestType::is_signed) {
    test_division(-1, 1.0, -1);
    test_division(1, -1.0, -1);
    test_division(-1, -1.0, 1);
    test_division(0, -1.0, 0);
    test_division(-110, 10.0, -11);
    test_division(110, -10.0, -11);
    test_division(-110, -10.0, 11);

    test_division(11, -0.5, -22);
    test_division(-11, 0.5, -22);
    test_division(-11, -0.5, 22);
  }
}

TEST_CASE_TEMPLATE("fixed-point divide by fixed point", TestType, s16_16, s24_8, s8_24, u16_16, u24_8, u8_24) {
  with_all([](auto b) {
    test_division(TestType(123), decltype(b)(1), 123);
    test_division(TestType(34), decltype(b)(2), 17);
    test_division(TestType(34), decltype(b)(0.5), 68);
    test_division(TestType(0), decltype(b)(1), 0);
    test_division(TestType(0), decltype(b)(0.5f), 0);

    if constexpr (TestType::is_signed) {
      test_division(TestType(-1), decltype(b)(1), -1);
      if constexpr (decltype(b)::is_signed) {
        test_division(TestType(1), decltype(b)(-1), -1);
        test_division(TestType(-1), decltype(b)(-1), 1);
        test_division(TestType(0), decltype(b)(-1), 0);
        test_division(TestType(-110), decltype(b)(-10), 11);
        test_division(TestType(110), decltype(b)(-10), -11);
        test_division(TestType(-11), decltype(b)(-0.5), 22);
        test_division(TestType(11), decltype(b)(-0.5), -22);
      }
      test_division(TestType(-110), decltype(b)(10), -11);
      test_division(TestType(-11), decltype(b)(0.5f), -22);
    }
  });
}
