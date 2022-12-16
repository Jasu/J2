#include <detail/preamble.hpp>

#include "properties/access/float_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"

TEST_CASE("double access read") {
  double i = 67.0;
  j::properties::access::float_access a{i};
  REQUIRE(a.get_value() == 67.0);
}

TEST_CASE("double access set_value") {
  double i = 67.0;
  j::properties::access::float_access a{i};
  REQUIRE(a.get_value() == 67.0);
  a.set_value(-1929.0);
  REQUIRE(a.get_value() == -1929.0);
  REQUIRE(i == -1929.0);
}

TEST_CASE("double reference get_value") {
  double i = 67.0;
  auto w = j::properties::wrappers::access(i);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::FLOAT);
  REQUIRE(w.is_float());
  REQUIRE(w.as_float().get_value() == 67.0);
}

TEST_CASE("double reference set_value") {
  double i = 67.0;
  auto w = j::properties::wrappers::access(i);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::FLOAT);
  REQUIRE(w.is_float());
  w.as_float().set_value(-1929.0);
  REQUIRE(i == -1929.0);
}

TEST_CASE("variant double access read") {
  j::properties::property_variant v = 182.0;
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::FLOAT);
  REQUIRE(w.is_float());
  REQUIRE(w.as_float().get_value() == 182.0);
}

TEST_CASE("variant double access set_value") {
  j::properties::property_variant v = 182.0;
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::FLOAT);
  REQUIRE(w.is_float());
  w.as_float().set_value(189.0);
  REQUIRE(v.as<double>() == 189.0);
}

TEST_CASE("successful float conversions") {
  float f = 1.5f;
  double d = 9.5;

  float dst_f = 1.0f;
  double dst_d = 1.0;

  j::properties::access::float_access fa{f};
  j::properties::access::float_access da{d};
  j::properties::access::float_access dfa{dst_f};
  j::properties::access::float_access dda{dst_d};

  dfa.assign_from(fa);
  REQUIRE(dst_f == 1.5f);
  dst_f = 1.0f;

  dfa.assign_from(da);
  REQUIRE(dst_f == 9.5f);
  dst_f = 1.0f;

  dda.assign_from(fa);
  REQUIRE(dst_d == 1.5);
  dst_d = 1.0;

  dda.assign_from(da);
  REQUIRE(dst_d == 9.5);
}

template<typename To, typename From>
static void test_int_to_float_conversion(From from) {
  To to = -1.41;
  j::properties::access::float_access to_access(to);
  j::properties::access::int_access from_access(from);

  to_access.assign_from(from_access);
  REQUIRE(to == from);
}

TEST_CASE("int-to-float conversions") {
  test_int_to_float_conversion<float, i8_t>(0);
  test_int_to_float_conversion<float, i16_t>(0);
  test_int_to_float_conversion<float, i32_t>(0);
  test_int_to_float_conversion<float, i64_t>(0);
  test_int_to_float_conversion<float, u8_t>(0);
  test_int_to_float_conversion<float, u16_t>(0);
  test_int_to_float_conversion<float, u32_t>(0);
  test_int_to_float_conversion<float, u64_t>(0);

  test_int_to_float_conversion<float, i8_t>(I8_MAX);
  test_int_to_float_conversion<float, i16_t>(I16_MAX);
  test_int_to_float_conversion<float, i32_t>(2147483520);
  test_int_to_float_conversion<float, i64_t>(9223371487098961920LL);
  test_int_to_float_conversion<float, u8_t>(U8_MAX);
  test_int_to_float_conversion<float, u16_t>(U16_MAX);
  test_int_to_float_conversion<float, u32_t>(4294967040);
  test_int_to_float_conversion<float, u64_t>(18446742974197923840ULL);

  test_int_to_float_conversion<float, i8_t>(I8_MIN);
  test_int_to_float_conversion<float, i16_t>(I16_MIN);
  test_int_to_float_conversion<float, i32_t>(-2147483520);
  test_int_to_float_conversion<float, i64_t>(-9223371487098961920LL);

  test_int_to_float_conversion<double, i8_t>(I8_MAX);
  test_int_to_float_conversion<double, i16_t>(I16_MAX);
  test_int_to_float_conversion<double, i32_t>(2147483520);
  test_int_to_float_conversion<double, i64_t>(9223372036854774784LL);
  test_int_to_float_conversion<double, u8_t>(U8_MAX);
  test_int_to_float_conversion<double, u16_t>(U16_MAX);
  test_int_to_float_conversion<double, u32_t>(4294967040);
  test_int_to_float_conversion<double, u64_t>(18446744073709549568ULL);

  test_int_to_float_conversion<double, i8_t>(I8_MIN);
  test_int_to_float_conversion<double, i16_t>(I16_MIN);
  test_int_to_float_conversion<double, i32_t>(-2147483520);
  test_int_to_float_conversion<double, i64_t>(-9223372036854774784LL);
}
