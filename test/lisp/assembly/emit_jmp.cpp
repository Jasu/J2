#include "common.hpp"

extern "C" {
  extern const u8_t jmp_rel_fixture[];
  extern const u8_t jmp_reg_fixture[];
  extern const u8_t jmp_mem_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Jmp");

TEST_CASE("Jmp imm8/32") {
  u32_t f = 0U;
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm8(0));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm8(1));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm8(-1));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm8(127));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm8(-128));

  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm32(128));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm32(-129));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm32(+0x81001));
  test_emit(get_test_fixture(jmp_rel_fixture, f++), i::jmp, a::imm32(-0x98765));
}

TEST_CASE("Jmp r64") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(jmp_reg_fixture, f++), i::jmp, r);
    });
}

TEST_CASE("Jmp mem") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(jmp_mem_fixture, f++), i::jmp, a::mem64(r));
    });

  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(jmp_mem_fixture, f++), i::jmp, a::mem64(r, x::rax));
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(jmp_mem_fixture, f++), i::jmp, a::mem64(r, x::r8, a::scale(2)));
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(jmp_mem_fixture, f++), i::jmp, a::mem64(r, 178));
    });
}

TEST_SUITE_END();
