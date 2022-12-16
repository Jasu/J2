#include "common.hpp"

extern "C" {
  extern const u8_t pop_r64_fixture[];
  extern const u8_t pop_m64_fixture[];
  extern const u8_t pop_r16_fixture[];
  extern const u8_t pop_m16_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit pop");

TEST_CASE("Pop r64") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(pop_r64_fixture, f++), i::pop, r);
    });
}

TEST_CASE("push r16") {
  u32_t f = 0U;
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(pop_r16_fixture, f++), i::pop, r);
    });
}

TEST_CASE("push m64") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(pop_m64_fixture, f++), i::pop, a::mem64(r));
    });
}

TEST_CASE("push m16") {
  u32_t f = 0U;
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(pop_m16_fixture, f++), i::pop, a::mem16(r));
    });
}

TEST_SUITE_END();
