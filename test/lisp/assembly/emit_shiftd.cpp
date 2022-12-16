#include "common.hpp"

extern "C" {
  extern const u8_t shld_fixture[];
  extern const u8_t shrd_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit SHLD/SHRD");

TEST_CASE("SHLD/SHRD") {
  u32_t f = 0U;
  for (u32_t i = 16; i <= 64; i += i) {
    with_reg(i, [&](a::reg rhs)  {
      with_reg(i, [&](a::reg lhs)  {
        test_emit(get_test_fixture(shld_fixture, f), i::shld, lhs, rhs, a::imm8(3));
        test_emit(get_test_fixture(shrd_fixture, f), i::shrd, lhs, rhs, a::imm8(3));
        ++f;
      });
      with_reg64([&](a::reg lhs)  {
        test_emit(get_test_fixture(shld_fixture, f), i::shld, a::memop(a::width(i), lhs), rhs, a::imm8(3));
        test_emit(get_test_fixture(shrd_fixture, f), i::shrd, a::memop(a::width(i), lhs), rhs, a::imm8(3));
        ++f;
      });
    });
    with_reg(i, [&](a::reg rhs)  {
      with_reg(i, [&](a::reg lhs)  {
        test_emit(get_test_fixture(shld_fixture, f), i::shld, lhs, rhs, x::cl);
        test_emit(get_test_fixture(shrd_fixture, f), i::shrd, lhs, rhs, x::cl);
        ++f;
      });
      with_reg64([&](a::reg lhs)  {
        test_emit(get_test_fixture(shld_fixture, f), i::shld, a::memop(a::width(i), lhs), rhs, x::cl);
        test_emit(get_test_fixture(shrd_fixture, f), i::shrd, a::memop(a::width(i), lhs), rhs, x::cl);
        ++f;
      });
    });
  }
}

TEST_SUITE_END();
