#include "common.hpp"

#include "strings/format.hpp"

extern "C" {
  extern const u8_t cmp_ri_fixture[];
  extern const u8_t cmp_mi_fixture[];
  extern const u8_t cmp_rax_fixture[];
  extern const u8_t cmp_mr_fixture[];

  extern const u8_t add_ri_fixture[];
  extern const u8_t add_mi_fixture[];
  extern const u8_t add_rax_fixture[];
  extern const u8_t add_mr_fixture[];

  extern const u8_t sub_ri_fixture[];
  extern const u8_t sub_mi_fixture[];
  extern const u8_t sub_rax_fixture[];
  extern const u8_t sub_mr_fixture[];

  extern const u8_t and_ri_fixture[];
  extern const u8_t and_mi_fixture[];
  extern const u8_t and_rax_fixture[];
  extern const u8_t and_mr_fixture[];

  extern const u8_t or_ri_fixture[];
  extern const u8_t or_mi_fixture[];
  extern const u8_t or_rax_fixture[];
  extern const u8_t or_mr_fixture[];

  extern const u8_t xor_ri_fixture[];
  extern const u8_t xor_mi_fixture[];
  extern const u8_t xor_rax_fixture[];
  extern const u8_t xor_mr_fixture[];

  extern const u8_t test_ri_fixture[];
  extern const u8_t test_mi_fixture[];
  extern const u8_t test_rax_fixture[];
  extern const u8_t test_mr_fixture[];
}

TEST_SUITE_BEGIN("Lisp - Assembly - Emit Cmp/Add/Sub");

namespace {
  struct J_TYPE_HIDDEN fixture_pair final {
    const u8_t * cmp;
    const u8_t * add;
    const u8_t * sub;
    const u8_t * band;
    const u8_t * bor;
    const u8_t * bxor;
    const u8_t * test;
  };

  const fixture_pair ri_fixture{
    .cmp = cmp_ri_fixture,
    .add = add_ri_fixture,
    .sub = sub_ri_fixture,
    .band = and_ri_fixture,
    .bor = or_ri_fixture,
    .bxor = xor_ri_fixture,
    .test = test_ri_fixture,
  };

  const fixture_pair rax_fixture{
    .cmp = cmp_rax_fixture,
    .add = add_rax_fixture,
    .sub = sub_rax_fixture,
    .band = and_rax_fixture,
    .bor = or_rax_fixture,
    .bxor = xor_rax_fixture,
    .test = test_rax_fixture,
  };

  const fixture_pair mi_fixture{
    .cmp = cmp_mi_fixture,
    .add = add_mi_fixture,
    .sub = sub_mi_fixture,
    .band = and_mi_fixture,
    .bor = or_mi_fixture,
    .bxor = xor_mi_fixture,
    .test = test_mi_fixture,
  };

  const fixture_pair mr_fixture{
    .cmp = cmp_mr_fixture,
    .add = add_mr_fixture,
    .sub = sub_mr_fixture,
    .band = and_mr_fixture,
    .bor = or_mr_fixture,
    .bxor = xor_mr_fixture,
    .test = test_mr_fixture,
  };

  J_ALWAYS_INLINE void do_test_cmp_add_sub(
    const fixture_pair & p,
    u32_t index,
    const a::operand & lhs,
    const a::operand & rhs,
    bool enable_test = true)
  {
    test_emit(get_test_fixture(p.cmp,  index), i::cmp,  lhs, rhs);
    test_emit(get_test_fixture(p.add,  index), i::add,  lhs, rhs);
    test_emit(get_test_fixture(p.sub,  index), i::sub,  lhs, rhs);
    test_emit(get_test_fixture(p.band, index), i::band, lhs, rhs);
    test_emit(get_test_fixture(p.bor,  index), i::bor,  lhs, rhs);
    test_emit(get_test_fixture(p.bxor, index), i::bxor,  lhs, rhs);
    if (enable_test) {
      test_emit(get_test_fixture(p.test, index), i::test,  lhs, rhs);
    }
  }
}

TEST_CASE("Cmp/Add/Sub r, imm") {
  u32_t f = 0U;
  with_all_gprs_immsize(
    [&f](a::reg r, a::width_t immsize) {
      do_test_cmp_add_sub(ri_fixture, f++, r, a::imm(immsize, 0));
    });
}

TEST_CASE("Cmp/Add/Sub m, 1") {
  u32_t f = 0U;
  with_size_immsize(
    [&](a::width_t w, a::width_t iw) {
      with_reg64(
        [&](a::reg r) {
          do_test_cmp_add_sub(mi_fixture, f++, a::memop(w, r), a::imm(iw, 1));
        });
    });
}

TEST_CASE("Cmp/Add/Sub rax/eax/ax/al, imm") {
  u32_t f = 0;
  with_size_immsize(
    [&f](j::lisp::mem_width size, a::width_t immsize) {
      const auto r = a::reg(x::preg::rax, size);
      do_test_cmp_add_sub(rax_fixture, f++, r, a::imm(immsize, 0));
      do_test_cmp_add_sub(rax_fixture, f++, r, a::imm(immsize, 1));
      do_test_cmp_add_sub(rax_fixture, f++, r, a::imm(immsize, -1));
      do_test_cmp_add_sub(rax_fixture, f++, r,
                  a::imm(immsize, a::max_signed_value(immsize)));
      do_test_cmp_add_sub(rax_fixture, f++, r,
                  a::imm(immsize, a::min_signed_value(immsize)));
    });
}

template<typename... MArgs>
static void mr_case(u32_t & f, MArgs && ... margs) {
  with_all_gprs_size(
    [&](a::reg src, j::lisp::mem_width w) {
      with_reg64(
        [&](a::reg base)  {
          do_test_cmp_add_sub(mr_fixture, f++, a::memop(w, base, static_cast<MArgs &&>(margs)...), src);
          do_test_cmp_add_sub(mr_fixture, f++, src, a::memop(w, base, static_cast<MArgs &&>(margs)...), false);
        });
    });
}

TEST_CASE("Cmp/Add/Sub [m], r") {
  u32_t f = 0U;
  mr_case(f);
  mr_case(f, 718);
  mr_case(f, x::rax, a::scale(2));
}


TEST_SUITE_END();
