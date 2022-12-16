#include "common.hpp"

extern "C" {
  extern const u8_t rcl_fixture[];
  extern const u8_t rcr_fixture[];
  extern const u8_t rol_fixture[];
  extern const u8_t ror_fixture[];
  extern const u8_t sal_fixture[];
  extern const u8_t sar_fixture[];
  extern const u8_t shl_fixture[];
  extern const u8_t shr_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Jcc");

struct J_TYPE_HIDDEN test_case {
  const u8_t * fixture;
  const x::instruction<12> & inst;
};

static const test_case fixtures[]{
  { rcl_fixture, i::rcl },
  { rcr_fixture, i::rcr },
  { rol_fixture, i::rol },
  { ror_fixture, i::ror },
  { sal_fixture, i::sal },
  { sar_fixture, i::sar },
  { shl_fixture, i::shl },
  { shr_fixture, i::shr },
};

TEST_CASE("Shift reg, 1/Cl") {
  u32_t f = 0U;
  with_all_gprs([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, r, a::imm8(1));
    }
    ++f;
  });

  with_all_gprs([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, r, x::cl);
    }
    ++f;
  });

  with_all_gprs([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, r, a::imm8(5));
    }
    ++f;
  });

  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem8(r), a::imm8(1));
    }
    ++f;
  });

  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem8(r), x::cl);
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem8(r), a::imm8(6));
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem16(r), a::imm8(1));
    }
    ++f;
  });

  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem16(r), x::cl);
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem16(r), a::imm8(6));
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem32(r), a::imm8(1));
    }
    ++f;
  });

  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem32(r), x::cl);
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem32(r), a::imm8(6));
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem64(r), a::imm8(1));
    }
    ++f;
  });

  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem64(r), x::cl);
    }
    ++f;
  });
  with_reg64([&](a::reg r)  {
    for (auto & fix : fixtures) {
      test_emit(get_test_fixture(fix.fixture, f), fix.inst, a::mem64(r), a::imm8(6));
    }
    ++f;
  });
}

TEST_SUITE_END();
