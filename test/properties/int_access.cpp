#include <detail/preamble.hpp>

#include "properties/access/int_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"

namespace {
  void assert_int_wrapper(j::properties::wrappers::wrapper & w, i64_t value) {
    REQUIRE_UNARY(w.type() == j::properties::PROPERTY_TYPE::INT);
    REQUIRE(w.is_int());
    REQUIRE(w.as_int().get_value() == value);
  }
}

TEST_CASE("int_t access read") {
  j::properties::int_t i = 67;
  j::properties::access::int_access a{i};
  REQUIRE(a.get_value() == 67);
}

TEST_CASE("int_t access set_value") {
  j::properties::int_t i = 67;
  j::properties::access::int_access a{i};
  REQUIRE(a.get_value() == 67);
  a.set_value(-1929);
  REQUIRE(a.get_value() == -1929);
  REQUIRE(i == -1929);
}

TEST_CASE("int_t reference read") {
  j::properties::int_t i = 67;
  auto w = j::properties::wrappers::access(i);
  assert_int_wrapper(w, 67);
}

TEST_CASE("int_t reference set_value") {
  j::properties::int_t i = 67;
  auto w = j::properties::wrappers::access(i);
  assert_int_wrapper(w, 67);
  w.as_int().set_value(1929);
  REQUIRE(i == 1929);
}

TEST_CASE("variant int_t access read") {
  j::properties::property_variant v = 182;
  auto w = j::properties::wrappers::access(v);
  assert_int_wrapper(w, 182);
}

TEST_CASE("variant int_t access set_value") {
  j::properties::property_variant v = 182;
  auto w = j::properties::wrappers::access(v);
  assert_int_wrapper(w, 182);
  w.as_int().set_value(9999);
  REQUIRE(w.as_int().get_value() == 9999);
  REQUIRE(v.as<j::properties::int_t>() == 9999);
}

template<typename To>
static void test_successful_int_conversion(j::properties::access::int_access & from_a, To val) {
  To to = 11;
  j::properties::access::int_access(to).assign_from(from_a);
  REQUIRE_UNARY(to == val);
}

template<typename From, typename... Tos>
J_ALWAYS_INLINE static void test_successful_int_conversions(From from) {
  j::properties::access::int_access from_a(from);
  (test_successful_int_conversion<Tos>(from_a, from), ...);
}

TEST_CASE("Successful int-to-int conversions") {
  test_successful_int_conversions<u8_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<i8_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<u16_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<i16_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<u32_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<i32_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<u64_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);
  test_successful_int_conversions<i64_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(0);

  test_successful_int_conversions<u8_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<i8_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<u16_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<i16_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<u32_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<i32_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<u64_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);
  test_successful_int_conversions<i64_t, u8_t, u16_t, u32_t, u64_t, i8_t, i16_t, i32_t, i64_t>(I8_MAX);

  test_successful_int_conversions<u8_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<u16_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<i16_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<u32_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<i32_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<u64_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);
  test_successful_int_conversions<i64_t, u8_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(U8_MAX);

  test_successful_int_conversions<u16_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);
  test_successful_int_conversions<i16_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);
  test_successful_int_conversions<u32_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);
  test_successful_int_conversions<i32_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);
  test_successful_int_conversions<u64_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);
  test_successful_int_conversions<i64_t, u16_t, u32_t, u64_t, i16_t, i32_t, i64_t>(I16_MAX);

  test_successful_int_conversions<u16_t, u16_t, u32_t, u64_t, i32_t, i64_t>(U16_MAX);
  test_successful_int_conversions<u32_t, u16_t, u32_t, u64_t, i32_t, i64_t>(U16_MAX);
  test_successful_int_conversions<i32_t, u16_t, u32_t, u64_t, i32_t, i64_t>(U16_MAX);
  test_successful_int_conversions<u64_t, u16_t, u32_t, u64_t, i32_t, i64_t>(U16_MAX);
  test_successful_int_conversions<i64_t, u16_t, u32_t, u64_t, i32_t, i64_t>(U16_MAX);

  test_successful_int_conversions<u32_t, u32_t, u64_t, i32_t, i64_t>(I32_MAX);
  test_successful_int_conversions<i32_t, u32_t, u64_t, i32_t, i64_t>(I32_MAX);
  test_successful_int_conversions<u64_t, u32_t, u64_t, i32_t, i64_t>(I32_MAX);
  test_successful_int_conversions<i64_t, u32_t, u64_t, i32_t, i64_t>(I32_MAX);

  test_successful_int_conversions<u32_t, u32_t, u64_t, i64_t>(U32_MAX);
  test_successful_int_conversions<u64_t, u32_t, u64_t, i64_t>(U32_MAX);
  test_successful_int_conversions<i64_t, u32_t, u64_t, i64_t>(U32_MAX);

  test_successful_int_conversions<u64_t, u64_t, i64_t>(I32_MAX);
  test_successful_int_conversions<i64_t, u64_t, i64_t>(I32_MAX);

  test_successful_int_conversions<u64_t, u64_t>(U64_MAX);

  test_successful_int_conversions<i8_t, i8_t, i16_t, i32_t, i64_t>(I8_MIN);
  test_successful_int_conversions<i16_t, i8_t, i16_t, i32_t, i64_t>(I8_MIN);
  test_successful_int_conversions<i32_t, i8_t, i16_t, i32_t, i64_t>(I8_MIN);
  test_successful_int_conversions<i64_t, i8_t, i16_t, i32_t, i64_t>(I8_MIN);
  test_successful_int_conversions<i16_t, i16_t, i32_t, i64_t>(I16_MIN);
  test_successful_int_conversions<i32_t, i16_t, i32_t, i64_t>(I16_MIN);
  test_successful_int_conversions<i64_t, i16_t, i32_t, i64_t>(I16_MIN);
  test_successful_int_conversions<i32_t, i32_t, i64_t>(I32_MIN);
  test_successful_int_conversions<i64_t, i32_t, i64_t>(I32_MIN);
  test_successful_int_conversions<i64_t, i64_t>(I64_MIN);
}


template<typename To>
static void test_successful_float_to_int_conversion(const j::properties::access::float_access & from_a, To expected) {
  To to = 77;
  j::properties::access::int_access{to}.assign_from(from_a);
  REQUIRE_UNARY(to == expected);
}

TEST_CASE("Successful float-to-int conversions") {
  float fz = 0.0f;
  j::properties::access::float_access from_float_z{fz};
  test_successful_float_to_int_conversion<i8_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<i16_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<i32_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<i64_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<u8_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<u16_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<u32_t>(from_float_z, 0);
  test_successful_float_to_int_conversion<u64_t>(from_float_z, 0);

  double dz = 0.0f;
  j::properties::access::float_access from_double_z{dz};
  test_successful_float_to_int_conversion<i8_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<i16_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<i32_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<i64_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<u8_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<u16_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<u32_t>(from_double_z, 0);
  test_successful_float_to_int_conversion<u64_t>(from_double_z, 0);


  float fnz = -0.0f;
  j::properties::access::float_access from_float_nz{fnz};
  test_successful_float_to_int_conversion<i8_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<i16_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<i32_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<i64_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<u8_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<u16_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<u32_t>(from_float_nz, 0);
  test_successful_float_to_int_conversion<u64_t>(from_float_nz, 0);

  double fdz = -0.0f;
  j::properties::access::float_access from_double_nz{fdz};
  test_successful_float_to_int_conversion<i8_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<i16_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<i32_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<i64_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<u8_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<u16_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<u32_t>(from_double_nz, 0);
  test_successful_float_to_int_conversion<u64_t>(from_double_nz, 0);

  test_successful_float_to_int_conversion<i8_t>(127.0f, 127);
  test_successful_float_to_int_conversion<i8_t>(127.0, 127);
  test_successful_float_to_int_conversion<i8_t>(-128.0f, -128);
  test_successful_float_to_int_conversion<i8_t>(-128.0, -128);

  test_successful_float_to_int_conversion<i16_t>(32767.0f, 32767);
  test_successful_float_to_int_conversion<i16_t>(32767.0, 32767);
  test_successful_float_to_int_conversion<i16_t>(-32768.0f, -32768);
  test_successful_float_to_int_conversion<i16_t>(-32768.0, -32768);

  test_successful_float_to_int_conversion<i32_t>(2147483647.0, 2147483647);
  test_successful_float_to_int_conversion<i32_t>(-2147483648.0, -2147483648);

  test_successful_float_to_int_conversion<u8_t>(255.0f, 255);
  test_successful_float_to_int_conversion<u8_t>(255.0, 255);

  test_successful_float_to_int_conversion<u16_t>(65535.0f, 65535);
  test_successful_float_to_int_conversion<u16_t>(65535.0, 65535);

  test_successful_float_to_int_conversion<u32_t>(4294967295.0, 4294967295UL);
  test_successful_float_to_int_conversion<u64_t>(4294967295.0, 4294967295UL);

  test_successful_float_to_int_conversion<i32_t>(8388607.0f, 8388607);
  test_successful_float_to_int_conversion<i32_t>(-8388607.0f, -8388607);
  test_successful_float_to_int_conversion<u32_t>(8388607.0f, 8388607);
  test_successful_float_to_int_conversion<i64_t>(8388607.0f, 8388607);
  test_successful_float_to_int_conversion<i64_t>(-8388607.0f, -8388607);
  test_successful_float_to_int_conversion<u64_t>(8388607.0f, 8388607);
  test_successful_float_to_int_conversion<i64_t>(4503599627370495.0, 4503599627370495LL);
  test_successful_float_to_int_conversion<i64_t>(-4503599627370495.0, -4503599627370495LL);
  test_successful_float_to_int_conversion<u64_t>(4503599627370495.0, 4503599627370495ULL);

  // Largest accurately representable integers
  test_successful_float_to_int_conversion<u32_t>(4294967040.0f, 4294967040U);
  test_successful_float_to_int_conversion<i32_t>(2147483520.0f, 2147483520);
  test_successful_float_to_int_conversion<i32_t>(-2147483520.0f, -2147483520);
  test_successful_float_to_int_conversion<u64_t>(18446742974197923840.0f, 18446742974197923840ULL);
  test_successful_float_to_int_conversion<i64_t>(9223371487098961920.0f, 9223371487098961920LL);
  test_successful_float_to_int_conversion<i64_t>(-9223371487098961920.0f, -9223371487098961920LL);

  test_successful_float_to_int_conversion<u64_t>(18446744073709549568.0, 18446744073709549568ULL);
  test_successful_float_to_int_conversion<i64_t>(9223372036854774784.0, 9223372036854774784LL);
  test_successful_float_to_int_conversion<i64_t>(-9223372036854774784.0, -9223372036854774784LL);
}
