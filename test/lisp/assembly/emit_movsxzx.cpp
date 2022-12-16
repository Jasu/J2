#include "common.hpp"

extern "C" {
  extern const u8_t movsx_fixture[];
  extern const u8_t movzx_fixture[];
  extern const u8_t movsxd_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Movd/Movq");

TEST_CASE("Movsx/zx") {
  u32_t f = 0U;
  with_reg_reg(16, 8, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, rhs);
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, rhs);
    ++f;
  });
  with_reg_reg(32, 8, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, rhs);
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, rhs);
    ++f;
  });
  with_reg_reg(64, 8, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, rhs);
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, rhs);
    ++f;
  });
  with_reg_reg(32, 16, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, rhs);
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, rhs);
    ++f;
  });
  with_reg_reg(64, 16, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, rhs);
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, rhs);
    ++f;
  });


  with_reg_reg(16, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, a::mem8(rhs));
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, a::mem8(rhs));
    ++f;
  });
  with_reg_reg(32, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, a::mem8(rhs));
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, a::mem8(rhs));
    ++f;
  });
  with_reg_reg(64, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, a::mem8(rhs));
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, a::mem8(rhs));
    ++f;
  });
  with_reg_reg(32, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, a::mem16(rhs));
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, a::mem16(rhs));
    ++f;
  });
  with_reg_reg(64, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsx_fixture, f), i::movsx, lhs, a::mem16(rhs));
    test_emit(get_test_fixture(movzx_fixture, f), i::movzx, lhs, a::mem16(rhs));
    ++f;
  });
}

TEST_CASE("Movsxd") {
  u32_t f = 0U;
  with_reg_reg(64, 32, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsxd_fixture, f++), i::movsxd, lhs, rhs);
  });

  with_reg_reg(64, 64, [&](a::reg lhs, a::reg rhs) {
    test_emit(get_test_fixture(movsxd_fixture, f++), i::movsxd, lhs, a::mem32(rhs));
  });
}


TEST_SUITE_END();
