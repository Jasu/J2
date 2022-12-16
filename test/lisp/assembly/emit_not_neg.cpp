#include "common.hpp"

extern "C" {
  extern const u8_t not_fixture[];
  extern const u8_t neg_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Not / Neg");

TEST_CASE("Not/Neg") {
  u32_t f = 0U;
  with_all_gprs([&](a::reg r)  {
    test_emit(get_test_fixture(not_fixture, f), i::bnot, r);
    test_emit(get_test_fixture(neg_fixture, f), i::neg, r);
    ++f;
  });
  for (auto w : a::widths) {
    with_reg64(
      [&](a::reg r) {
        test_emit(get_test_fixture(not_fixture, f), i::bnot, a::memop(w, r));
        test_emit(get_test_fixture(neg_fixture, f), i::neg, a::memop(w, r));
        ++f;
        test_emit(get_test_fixture(not_fixture, f), i::bnot, a::memop(w, r, x::rax));
        test_emit(get_test_fixture(neg_fixture, f), i::neg, a::memop(w, r, x::rax));
        ++f;
        test_emit(get_test_fixture(not_fixture, f), i::bnot, a::memop(w, r, x::r14, a::scale(4), -10));
        test_emit(get_test_fixture(neg_fixture, f), i::neg, a::memop(w, r, x::r14, a::scale(4), -10));
        ++f;
      });
  }
}


TEST_SUITE_END();
