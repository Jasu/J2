#pragma once

#include "exceptions/assert_lite.hpp"
#include "lisp/common/mem_width.hpp"

namespace j::lisp::inline common {
  enum class mem_scale : u8_t {
    none = 0U,

    byte,
    word,
    dword,
    qword,

    width,

    scale_1 = mem_scale::byte,
    scale_2 = mem_scale::word,
    scale_4 = mem_scale::dword,
    scale_8 = mem_scale::qword,
  };

  [[nodiscard]] inline constexpr mem_scale bits_to_scale(u8_t bits) noexcept {
    switch(bits) {
    case 8:  return mem_scale::byte;
    case 16: return mem_scale::word;
    case 32: return mem_scale::dword;
    case 64: return mem_scale::qword;
    default: J_FAIL("Width out of range.");
    }
  }

  [[nodiscard]] inline constexpr enum mem_scale bytes_to_scale(u8_t i) noexcept {
    switch(i) {
    case 1: return mem_scale::byte;
    case 2: return mem_scale::word;
    case 4: return mem_scale::dword;
    case 8: return mem_scale::qword;
    default: J_ASSERT_FAIL("Scale out of range.");
    }
  }

  [[nodiscard]] inline constexpr u8_t scale_to_bytes(mem_scale scale) noexcept {
    switch(scale) {
    case mem_scale::byte:  return 1U;
    case mem_scale::word:  return 2U;
    case mem_scale::dword: return 4U;
    case mem_scale::qword: return 8U;
    case mem_scale::width:
    case mem_scale::none:  J_FAIL("Width out of range.");
    }
  }

  [[nodiscard]] inline constexpr mem_scale normalize_scale(mem_scale scale, mem_width width) noexcept {
    J_ASSUME(scale != mem_scale::none);
    return scale == mem_scale::width ? (mem_scale)width : scale;
  }

  [[nodiscard]] inline constexpr u8_t scale_to_bytes(mem_scale scale, mem_width width) noexcept {
    return scale_to_bytes(normalize_scale(scale, width));
  }

  [[nodiscard]] inline constexpr u8_t scale_bits(mem_scale scale, mem_width width) noexcept
  { return scale_to_bytes(scale, width) * 8U; }
}
