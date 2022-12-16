#include <detail/preamble.hpp>

#include "strings/format.hpp"
#include "strings/formatting/format_digits.hpp"
#include "hzd/type_traits.hpp"

TEST_SUITE_BEGIN("Strings - Format Integer");

namespace s = j::strings;

TEST_CASE_TEMPLATE("Basic", T, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t) {
  REQUIRE(s::format("{}", T(0)) == "0");
  REQUIRE(s::format("{:x}", T(0)) == "0");
  REQUIRE(s::format("{:X}", T(0)) == "0");
  REQUIRE(s::format("{:b}", T(0)) == "0");

  REQUIRE(s::format("{}", T(1)) == "1");
  REQUIRE(s::format("{:x}", T(1)) == "1");
  REQUIRE(s::format("{:X}", T(1)) == "1");
  REQUIRE(s::format("{:b}", T(1)) == "1");

  if constexpr (::j::is_signed_v<T>) {
    REQUIRE(s::format("{}", T(-1)) == "-1");
    REQUIRE(s::format("{:x}", T(-1)) == "-1");
    REQUIRE(s::format("{:X}", T(-1)) == "-1");
    REQUIRE(s::format("{:b}", T(-1)) == "-1");
  }
}

TEST_CASE_TEMPLATE("Binary", T, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t) {
  const char top_bit[] =
    "10000000000000000000000000000000"
    "00000000000000000000000000000000";
  static_assert(sizeof(top_bit) == 65);
  const u32_t max_digits = sizeof(T) * 8 - (::j::is_signed_v<T> ? 1U : 0U);
  T v = 1;
  for (u32_t i = 0; i < max_digits; ++i, v <<= 1) {
    REQUIRE(s::num_binary_digits(v) == i + 1);
    REQUIRE(s::format("{:b}", v) == s::const_string_view(top_bit, i + 1));
  }
}

TEST_SUITE_END();
