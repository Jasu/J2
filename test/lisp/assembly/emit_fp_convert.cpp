#include "common.hpp"

extern "C" {
  extern const u8_t si_fixture[];
  extern const u8_t sd_ss_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit FP-conversions");

TEST_CASE("SI <-> SD") {
  u32_t f = 0U;
    with_reg32(
      [&](a::reg si) {
        with_xmms(
          [&](a::reg sd) {
            test_emit(get_test_fixture(si_fixture, f++), i::cvtsi2ss, sd, si);
            test_emit(get_test_fixture(si_fixture, f++), i::cvtss2si, si, sd);
            test_emit(get_test_fixture(si_fixture, f++), i::cvtsi2sd, sd, si);
            test_emit(get_test_fixture(si_fixture, f++), i::cvtsd2si, si, sd);
          });
      });
}

TEST_CASE("SS <-> SD") {
  u32_t f = 0U;
    with_xmms(
      [&](a::reg xmma) {
        with_xmms(
          [&](a::reg xmmb) {
            test_emit(get_test_fixture(sd_ss_fixture, f++), i::cvtsd2ss, xmmb, xmma);
            test_emit(get_test_fixture(sd_ss_fixture, f++), i::cvtss2sd, xmmb, xmma);
          });
      });
}


TEST_SUITE_END();
