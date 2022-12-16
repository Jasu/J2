#include "common.hpp"

extern "C" {
  extern const u8_t ucomiss_fixture[];
  extern const u8_t ucomisd_fixture[];
  extern const u8_t subss_fixture[];
  extern const u8_t subsd_fixture[];
  extern const u8_t addss_fixture[];
  extern const u8_t addsd_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit FP-comparison");

namespace {
  struct J_TYPE_HIDDEN test_case final {
    const u8_t * fixture;
    bool is_double;
    x::instruction_encodings inst;
  };

  const test_case cases[]{
    { ucomiss_fixture, false, i::ucomiss },
    { ucomisd_fixture, true,  i::ucomisd },
    { subss_fixture,   false, i::subss },
    { subsd_fixture,   true,  i::subsd },
    { addss_fixture,   false, i::addss },
    { addsd_fixture,   true,  i::addsd },
  };
}

TEST_CASE("ucomis[sd], subs[sd], adds[sd]") {
  u32_t f = 0U;
    with_xmms(
      [&](a::reg rhs) {
        with_xmms(
          [&](a::reg lhs) {
            for (auto & c : cases) {
              test_emit(get_test_fixture(c.fixture, f), c.inst, lhs, rhs);
            }
            ++f;
          });
      });
    with_reg64(
      [&](a::reg rhs) {
        with_xmms(
          [&](a::reg lhs) {
            for (auto & c : cases) {
              test_emit(get_test_fixture(c.fixture, f), c.inst, lhs,
                        c.is_double ? a::mem64(rhs) : a::mem32(rhs));
            }
            ++f;
          });
      });
}
