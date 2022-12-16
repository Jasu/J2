#include "common.hpp"

extern "C" {
  extern const u8_t push_r64_fixture[];
  extern const u8_t push_m64_fixture[];
  extern const u8_t push_r16_fixture[];
  extern const u8_t push_m16_fixture[];
  extern const u8_t push_imm_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit push");

TEST_CASE("push r64") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(push_r64_fixture, f++), i::push, r);
    });
}

TEST_CASE("push r16") {
  u32_t f = 0U;
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(push_r16_fixture, f++), i::push, r);
    });
}

TEST_CASE("push m64") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(push_m64_fixture, f++), i::push, a::mem64(r));
    });
}

TEST_CASE("push m16") {
  u32_t f = 0U;
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(push_m16_fixture, f++), i::push, a::mem16(r));
    });
}

TEST_CASE("push imm8/16/32") {
  u32_t f = 0U;
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm8(0));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm8(I8_MIN));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm8(I8_MAX));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm16(0));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm16(I16_MIN));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm16(I16_MAX));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm32(0));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm32(I32_MIN));
  test_emit(get_test_fixture(push_imm_fixture, f++), i::push, a::imm32(I32_MAX));
}

TEST_SUITE_END();
