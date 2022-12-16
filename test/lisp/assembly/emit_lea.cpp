#include "common.hpp"

extern "C" {
  extern const u8_t lea_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Lea");

template<typename... Args>
static void test_lea64(u32_t & f, Args && ... args) {
  with_reg64(
    [&](a::reg base) {
      with_reg64(
        [&](a::reg dst) {
          test_emit(get_test_fixture(lea_fixture, f++), i::lea, dst, a::mem32(base, static_cast<Args &&>(args)...));
        });
    });
}

template<typename... Args>
static void test_lea32(u32_t & f, Args && ... args) {
  with_reg64(
    [&](a::reg base) {
      with_reg32(
        [&](a::reg dst) {
          test_emit(get_test_fixture(lea_fixture, f++), i::lea, dst, a::mem32(base, static_cast<Args &&>(args)...));
        });
    });
}

TEST_CASE("Lea r64/32, mem") {
  u32_t f = 0U;
  test_lea64(f, 10);
  test_lea64(f, x::rbx);
  test_lea64(f, x::rax, a::scale(2));
  test_lea64(f, x::rbp, a::scale(4), 81);
  test_lea64(f, x::r15, a::scale(8), - 1111);

  test_lea32(f, 10);
  test_lea32(f, x::ebx);
  test_lea32(f, x::eax, a::scale(2));
  test_lea32(f, x::ebp, a::scale(4), 81);
  test_lea32(f, x::r15d, a::scale(8), - 1111);
}

TEST_SUITE_END();
