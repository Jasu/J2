#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::lisp::inline common {
  enum class mem_width : u8_t {
    none = 0U,

    byte,
    word,
    dword,
    qword,

    vec128,
    vec256,
    vec512,
  };

  J_A(ND) constexpr inline mem_width scalar_widths[]{
    mem_width::byte,
    mem_width::word,
    mem_width::dword,
    mem_width::qword,
  };

  [[nodiscard]] inline constexpr mem_width width(u8_t bits) noexcept {
    switch(bits) {
    case 8:  return mem_width::byte;
    case 16: return mem_width::word;
    case 32: return mem_width::dword;
    case 64: return mem_width::qword;
    case 128: return mem_width::vec128;
    case 256: return mem_width::vec256;
    case 512: return mem_width::vec512;
    default: J_FAIL("Width out of range.");
    }
  }

  J_A(AI,NODISC) inline constexpr i32_t width_bytes(mem_width w) noexcept {
    return w == mem_width::none ? 0 : (1 << ((i32_t)w - 1));
  }

  J_A(AI,NODISC) inline constexpr i32_t width_bits(mem_width w) noexcept {
    return w == mem_width::none ? 0 : (8 << ((i32_t)w - 1));
  }

  [[nodiscard]] inline constexpr i64_t min_signed_value(mem_width w) noexcept {
    switch(w) {
    case mem_width::byte:  return I8_MIN;
    case mem_width::word:  return I16_MIN;
    case mem_width::dword: return I32_MIN;
    case mem_width::qword: return I64_MIN;
    default: J_FAIL("Width out of range.");
    }
  }

  [[nodiscard]] inline constexpr u64_t max_unsigned_value(mem_width w) noexcept {
    switch(w) {
    case mem_width::byte:  return U8_MAX;
    case mem_width::word:  return U16_MAX;
    case mem_width::dword: return U32_MAX;
    case mem_width::qword: return U64_MAX;
    default: J_FAIL("Width out of range.");
    }
  }
}
