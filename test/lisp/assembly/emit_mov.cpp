#include "common.hpp"

#include "strings/format.hpp"

extern "C" {
  /// Generic MOV test fixture, exported from mov.asm
  extern const u8_t mov_fixture[];
  /// Generic MOV test fixture, exported from mov_imm.asm
  extern const u8_t mov_imm_fixture[];
  /// Generic MOV test fixture, exported from mov_index.asm
  extern const u8_t mov_index_fixture[];
  /// Generic MOV test fixture, exported from mov_rip.asm
  extern const u8_t mov_rip_fixture[];
}


TEST_SUITE_BEGIN("Lisp - Assembly");

namespace {
  J_ALWAYS_INLINE void do_test_mov(j::strings::const_string_view expected,
                                   const a::operand & lhs,
                                   const a::operand & rhs)
  { test_emit(expected, i::mov, lhs, rhs); }

  J_ALWAYS_INLINE void test_mov(u32_t index, const a::operand & lhs, const a::operand & rhs) {
    do_test_mov(get_test_fixture(mov_fixture, index), lhs, rhs);
  }
  J_ALWAYS_INLINE void test_mov_imm(u32_t index, const a::operand & lhs, const a::operand & rhs) {
    do_test_mov(get_test_fixture(mov_imm_fixture, index), lhs, rhs);
  }
  J_ALWAYS_INLINE void test_mov_rip(u32_t index, const a::operand & lhs, const a::operand & rhs) {
    do_test_mov(get_test_fixture(mov_rip_fixture, index), lhs, rhs);
  }
  J_ALWAYS_INLINE void test_mov_index(u32_t index, const a::operand & lhs, const a::operand & rhs) {
    do_test_mov(get_test_fixture(mov_index_fixture, index), lhs, rhs);
  }
}

TEST_CASE("mov r64, r64") {
  u32_t f = 0U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, x::gpr_64[i], x::gpr_64[j]);
      ++f;
    }
  }
}

TEST_CASE("mov r32, r32") {
  u32_t f = 256U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      CAPTURE(i);
      CAPTURE(j);
      test_mov(f, x::gpr_32[i], x::gpr_32[j]);
      ++f;
    }
  }
}

TEST_CASE("mov r16, r16") {
  u32_t f = 512U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      CAPTURE(i);
      CAPTURE(j);
      test_mov(f, x::gpr_16[i], x::gpr_16[j]);
      ++f;
    }
  }
}

TEST_CASE("mov r8, r8") {
  u32_t f = 768U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      CAPTURE(i);
      CAPTURE(j);
      test_mov(f, x::gpr_8[i], x::gpr_8[j]);
      ++f;
    }
  }
}

TEST_CASE("mov r64, QWORD [r64]") {
  u32_t f = 1024U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, x::gpr_64[i], a::mem64(x::gpr_64[j]));
      ++f;
    }
  }
}

TEST_CASE("mov r32, DWORD [r64]") {
  u32_t f = 1024U + 256U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, x::gpr_32[i], a::mem32(x::gpr_64[j]));
      ++f;
    }
  }
}

TEST_CASE("mov r16, WORD [r64]") {
  u32_t f = 1024U + 512U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, x::gpr_16[i], a::mem16(x::gpr_64[j]));
      ++f;
    }
  }
}

TEST_CASE("mov r8, BYTE [r64]") {
  u32_t f = 1024U + 768U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, x::gpr_8[i], a::mem8(x::gpr_64[j]));
      ++f;
    }
  }
}

TEST_CASE("mov QWORD [r64], r64") {
  u32_t f = 2048U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, a::mem64(x::gpr_64[i]), x::gpr_64[j]);
      ++f;
    }
  }
}

TEST_CASE("mov DWORD [r64], r32") {
  u32_t f = 2048U + 256U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, a::mem32(x::gpr_64[i]), x::gpr_32[j]);
      ++f;
    }
  }
}

TEST_CASE("mov WORD [r64], r16") {
  u32_t f = 2048U + 512U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, a::mem16(x::gpr_64[i]), x::gpr_16[j]);
      ++f;
    }
  }
}

TEST_CASE("mov BYTE [r64], r8") {
  u32_t f = 2048U + 768U;
  for (u32_t i = 0U; i < 16U; ++i) {
    for (u32_t j = 0U; j < 16U; ++j) {
      test_mov(f, a::mem8(x::gpr_64[i]), x::gpr_8[j]);
      ++f;
    }
  }
}

TEST_CASE("mov r64, QWORD 1") {
  u32_t f = 0;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, x::gpr_64[i], a::imm64(1));
    ++f;
  }
}

TEST_CASE("mov r64, DWORD 1") {
  u32_t f = 16;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, x::gpr_64[i], a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov r32, DWORD 1") {
  u32_t f = 32;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, x::gpr_32[i], a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov r16, WORD 1") {
  u32_t f = 48;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, x::gpr_16[i], a::imm16(1));
    ++f;
  }
}

TEST_CASE("mov r8, BYTE 1") {
  u32_t f = 64;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, x::gpr_8[i], a::imm8(1));
    ++f;
  }
}

TEST_CASE("mov QWORD [r64], DWORD 1") {
  u32_t f = 80;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem64(x::gpr_64[i]), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov DWORD [r64], DWORD 1") {
  u32_t f = 96;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem32(x::gpr_64[i]), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov WORD [r64], WORD 1") {
  u32_t f = 112;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem16(x::gpr_64[i]), a::imm16(1));
    ++f;
  }
}

TEST_CASE("mov BYTE [r64], BYTE 1") {
  u32_t f = 128;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem8(x::gpr_64[i]), a::imm8(1));
    ++f;
  }
}

TEST_CASE("mov QWORD [r64 + 123], DWORD 1") {
  u32_t f = 144;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem64(x::gpr_64[i], 123), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov DWORD [r64 + 123], DWORD 1") {
  u32_t f = 160;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem32(x::gpr_64[i], 123), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov WORD [r64 + 123], WORD 1") {
  u32_t f = 176;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem16(x::gpr_64[i], 123), a::imm16(1));
    ++f;
  }
}

TEST_CASE("mov BYTE [r64 + 123], BYTE 1") {
  u32_t f = 192;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem8(x::gpr_64[i], 123), a::imm8(1));
    ++f;
  }
}

TEST_CASE("mov QWORD [r64 + 991291], DWORD 1") {
  u32_t f = 208;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem64(x::gpr_64[i], 991291), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov DWORD [r64 + 991291], DWORD 1") {
  u32_t f = 224;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem32(x::gpr_64[i], 991291), a::imm32(1));
    ++f;
  }
}

TEST_CASE("mov WORD [r64 + 991291], WORD 1") {
  u32_t f = 240;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem16(x::gpr_64[i], 991291), a::imm16(1));
    ++f;
  }
}

TEST_CASE("mov BYTE [r64 + 991291], BYTE 1") {
  u32_t f = 256;
  for (u32_t i = 0U; i < 16U; ++i) {
    test_mov_imm(f, a::mem8(x::gpr_64[i], 991291), a::imm8(1));
    ++f;
  }
}

TEST_CASE("mov [r64 + I*r64 + DISP], 1") {
  u32_t f = 272;
  const i32_t disps[]{ 0, 123, 1219919, 127, -128, -129, 128 };
  for (const i32_t disp : disps) {
    for (u32_t w = 8; w <= 64; w *= 2) {
      for (u32_t s = 1; s <= 8; s *= 2) {
        for (u32_t i = 0U; i < 16U; ++i) {
          if (i == 4) {
            continue;
          }
          for (u32_t j = 0U; j < 16U; ++j) {
            test_mov_imm(f,
                         a::memop(a::width(w), x::gpr_64[j], x::gpr_64[i], a::scale(s), disp),
                         a::imm(a::width(j::min(w, 32)), 1));
            ++f;
          }
        }
      }
    }
  }
}

TEST_CASE("mov r, [rip + disp]") {
  u32_t f = 0;
  const i32_t disps []{ 0, 123, 192919, -1, -9329188 };
  for (const i32_t disp : disps) {
    for (u32_t w = 8; w <= 64; w *= 2) {
      for (u32_t i = 0U; i < 16U; ++i) {
        test_mov_rip(f, a::gpr(i, a::width(w)), a::rel(a::width(w), disp));
        ++f;
      }
    }
  }
}

TEST_CASE("mov [rip + disp], r") {
  u32_t f = 320;
  const i32_t disps []{ 0, 123, 192919, -1, -9329188 };
  for (const i32_t disp : disps) {
    for (u32_t w = 8; w <= 64; w *= 2) {
      for (u32_t i = 0U; i < 16U; ++i) {
        test_mov_rip(f, a::rel(a::width(w), disp), a::gpr(i, a::width(w)));
        ++f;
      }
    }
  }
}

TEST_CASE("mov [rip + disp], imm") {
  u32_t f = 640;
  test_mov_rip(f++, a::rel64(7), a::imm32(1));
  test_mov_rip(f++, a::rel32(-8), a::imm32(2));
  test_mov_rip(f++, a::rel16(9), a::imm16(3));
  test_mov_rip(f++, a::rel8(-19), a::imm8(5));
}

TEST_CASE("mov [r + i*r + disp], r / swapped") {
  u32_t f = 0U;
  for (u32_t sz = 8U; sz <= 64U; sz *= 2U) {

    for (u32_t is_rm = 0U; is_rm != 2U; ++is_rm) {
      i32_t disp = 0U;
      if (sz == 8U && is_rm) {
        disp = 128;
      } else if (sz == 16U && !is_rm) {
        disp = -129;
      } else if (sz == 32U && !is_rm) {
        disp = -128;
      } else if (sz == 64U && is_rm) {
        disp = 127;
      }
      for (u32_t s = 1U; s <= 8U; s *= 2U) {
        for (u32_t i = 0U; i < 16U; ++i) {
          if (i == 4) {
            continue;
          }
          for (u32_t b = 0U; b < 16U; ++b) {
            const auto mem_op = a::memop(a::width(sz), a::gpr64(b), a::gpr64(i), a::scale(s), disp);
            for (u32_t r = 0U; r < 16U; ++r) {
              const auto reg_op = a::gpr(r, a::width(sz));
              if (is_rm) {
                test_mov_index(f++, reg_op, mem_op);
              } else {
                test_mov_index(f++, mem_op, reg_op);
              }
            }
          }
        }
      }
    }
  }
}
TEST_SUITE_END();
