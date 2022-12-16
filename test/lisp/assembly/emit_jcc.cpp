#include "common.hpp"

extern "C" {
  extern const u8_t ja_fixture[];
  extern const u8_t jae_fixture[];
  extern const u8_t jb_fixture[];
  extern const u8_t jbe_fixture[];
  extern const u8_t je_fixture[];
  extern const u8_t jg_fixture[];
  extern const u8_t jge_fixture[];
  extern const u8_t jl_fixture[];
  extern const u8_t jle_fixture[];
  extern const u8_t jne_fixture[];
  extern const u8_t jno_fixture[];
  extern const u8_t jnp_fixture[];
  extern const u8_t jns_fixture[];
  extern const u8_t jo_fixture[];
  extern const u8_t jp_fixture[];
  extern const u8_t js_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Jcc");

struct J_TYPE_HIDDEN test_case {
  const u8_t * fixture;
  const x::instruction<2> & inst;
};

static const test_case fixtures[]{
  { ja_fixture,  i::ja },
  { jae_fixture, i::jae },
  { jb_fixture,  i::jb },
  { jbe_fixture, i::jbe },
  { je_fixture,  i::je },
  { jg_fixture,  i::jg },
  { jge_fixture, i::jge },
  { jl_fixture,  i::jl },
  { jle_fixture, i::jle },
  { jne_fixture, i::jne },
  { jno_fixture, i::jno },
  { jnp_fixture, i::jnp },
  { jns_fixture, i::jns },
  { jo_fixture,  i::jo },
  { jp_fixture,  i::jp },
  { js_fixture,  i::js },
};

TEST_CASE("Jcc imm8/32") {
  for (auto & fix : fixtures) {
    u32_t f = 0U;
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm8(0));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm8(1));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm8(-1));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm8(127));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm8(-128));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm32(128));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm32(-129));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm32(+0x81001));
    test_emit(get_test_fixture(fix.fixture, f++), fix.inst, a::imm32(-0x98765));
  }
}

TEST_SUITE_END();
