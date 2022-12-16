#include "common.hpp"

extern "C" {
  extern const u8_t ret_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit ret");

TEST_CASE("Ret") {
  u32_t f = 0U;
  test_emit(get_test_fixture(ret_fixture, f++), i::ret);
  test_emit(get_test_fixture(ret_fixture, f++), i::ret, a::imm16(0));
  test_emit(get_test_fixture(ret_fixture, f++), i::ret, a::imm16(1));
  test_emit(get_test_fixture(ret_fixture, f++), i::ret, a::imm16(2));
  test_emit(get_test_fixture(ret_fixture, f++), i::ret, a::imm16(3));
  test_emit(get_test_fixture(ret_fixture, f++), i::ret, a::imm16(1024));
}

TEST_SUITE_END();
