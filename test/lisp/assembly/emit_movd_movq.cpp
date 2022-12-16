#include "common.hpp"

extern "C" {
  extern const u8_t movd_fixture[];
  extern const u8_t movq_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Movd/Movq");

TEST_CASE("Movq") {
  u32_t f = 0U;
  with_reg64(
    [&](a::reg r64) {
      with_xmms(
        [&](a::reg xmm) {
          CAPTURE(f);
          test_emit(get_test_fixture(movq_fixture, f++), i::movq, xmm, r64);
          CAPTURE(f);
          test_emit(get_test_fixture(movq_fixture, f++), i::movq, r64, xmm);
        });
    });

  with_reg64(
    [&](a::reg r64) {
      with_xmms(
        [&](a::reg xmm) {
          CAPTURE(f);
          test_emit(get_test_fixture(movq_fixture, f++), i::movq, xmm, a::mem64(r64));
          CAPTURE(f);
          test_emit(get_test_fixture(movq_fixture, f++), i::movq, a::mem64(r64), xmm);
        });
    });
}
TEST_CASE("Movd") {
  u32_t f = 0U;
  with_reg32(
    [&](a::reg r32) {
      with_xmms(
        [&](a::reg xmm) {
          CAPTURE(f);
          test_emit(get_test_fixture(movd_fixture, f++), i::movd, xmm, r32);
          CAPTURE(f);
          test_emit(get_test_fixture(movd_fixture, f++), i::movd, r32, xmm);
        });
    });
  with_reg64(
    [&](a::reg r64) {
      with_xmms(
        [&](a::reg xmm) {
          CAPTURE(f);
          test_emit(get_test_fixture(movd_fixture, f++), i::movd, xmm, a::mem32(r64));
          CAPTURE(f);
          test_emit(get_test_fixture(movd_fixture, f++), i::movd, a::mem32(r64), xmm);
        });
    });
}


TEST_SUITE_END();
