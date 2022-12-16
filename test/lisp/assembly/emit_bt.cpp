#include "common.hpp"

extern "C" {
  extern const u8_t bt_reg_fixture[];
  extern const u8_t bt_mem_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit BT");

TEST_CASE("BT reg") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, x::rax);
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, x::r14);
    });
  with_reg32(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, x::eax);
    });
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, x::cx);
    });

  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, a::imm8(1));
    });
  with_reg32(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, a::imm8(2));
    });
  with_reg16(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_reg_fixture, f++), i::bt, r, a::imm8(4));
    });
}

TEST_CASE("BT mem") {
  u32_t f = 0U;
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem64(r), x::rax);
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem64(r), x::r14);
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem32(r), x::eax);
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem16(r), x::cx);
    });

  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem64(r), a::imm8(1));
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem32(r), a::imm8(2));
    });
  with_reg64(
    [&f](a::reg r) {
      test_emit(get_test_fixture(bt_mem_fixture, f++), i::bt, a::mem16(r), a::imm8(3));
    });
}


TEST_SUITE_END();
