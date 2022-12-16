
#include "common.hpp"

extern "C" {

  extern const u8_t seto_fixture[];
  extern const u8_t setno_fixture[];
  extern const u8_t setc_fixture[];
  extern const u8_t setnc_fixture[];
  extern const u8_t setz_fixture[];
  extern const u8_t setnz_fixture[];
  extern const u8_t setna_fixture[];
  extern const u8_t seta_fixture[];
  extern const u8_t sets_fixture[];
  extern const u8_t setns_fixture[];
  extern const u8_t setp_fixture[];
  extern const u8_t setnp_fixture[];
  extern const u8_t setl_fixture[];
  extern const u8_t setnl_fixture[];
  extern const u8_t setle_fixture[];
  extern const u8_t setnle_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Setcc");

struct J_TYPE_HIDDEN test_case {
  const u8_t * fixture;
  const x::instruction<1> & inst;
};

static const test_case fixtures[]{
  { seto_fixture, i::seto },
  { setno_fixture, i::setno },
  { setc_fixture, i::setc },
  { setnc_fixture, i::setnc },
  { setz_fixture, i::setz },
  { setnz_fixture, i::setnz },
  { setna_fixture, i::setna },
  { seta_fixture, i::seta },
  { sets_fixture, i::sets },
  { setns_fixture, i::setns },
  { setp_fixture, i::setp },
  { setnp_fixture, i::setnp },
  { setl_fixture, i::setl },
  { setnl_fixture, i::setnl },
  { setle_fixture, i::setle },
  { setnle_fixture, i::setnle },
};

TEST_CASE("Setcc r8/m8") {
  for (auto & fix : fixtures) {
    u32_t f = 0U;
    with_reg8([&](auto r) {
      test_emit(get_test_fixture(fix.fixture, f++), fix.inst, r);
    });
    with_reg64([&](auto r) {
      test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::mem8(r));
      test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::mem8(r, x::rax, a::scale(4)));
      test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::mem8(r, x::r11, a::scale(8), 71));
    });
  }
}

TEST_SUITE_END();
