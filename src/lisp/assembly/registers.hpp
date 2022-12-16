#pragma once

#include "lisp/common/mem_width.hpp"

#define PHYS_REG_MASK 31

namespace j::lisp::assembly {
  /// Physical register, e.g. the register backing `rax`, `eax`, `ax`, `al`, and `ah`.
  union phys_reg final {
    u8_t index = 32;

    J_A(AI,NODISC,HIDDEN,ND) inline constexpr bool operator==(phys_reg rhs) const noexcept {
      return index == rhs.index;
    }

    J_A(AI,NODISC,HIDDEN,ND) constexpr explicit operator bool() const noexcept
    { return index < 32; }
    J_A(AI,NODISC,HIDDEN,ND) constexpr bool operator!() const noexcept
    { return index == 32; }
    J_A(AI,NODISC,HIDDEN,ND) constexpr inline bool is_gpr() const noexcept
    { return index < 16; }
    J_A(AI,NODISC,HIDDEN,ND) constexpr bool is_fp() const noexcept
    { return index >= 16; }
  };

  union reg final {
    u8_t value = 0;

    J_A(AI,ND,NODISC,HIDDEN) inline phys_reg phys() const noexcept {
      return phys_reg{(u8_t)(value & PHYS_REG_MASK)};
    }

    J_A(AI,ND,NODISC,HIDDEN) inline u8_t index() const noexcept {
      return value & PHYS_REG_MASK;
    }

    J_A(AI,NODISC,ND,HIDDEN) inline constexpr bool operator==(reg rhs) const noexcept {
      return value == rhs.value;
    }

    J_A(AI,NODISC,ND,HIDDEN) inline constexpr bool operator!() const noexcept
    { return !value; }
    J_A(AI,NODISC,ND,HIDDEN) inline constexpr explicit operator bool() const noexcept
    { return value; }

    [[nodiscard]] inline constexpr reg with_width(mem_width w) const noexcept {
      return {(u8_t)((value & PHYS_REG_MASK) | ((u8_t)w << 5))};
    }

    J_A(AI,ND,HIDDEN) inline void set_width(mem_width w) noexcept {
      value = (value & PHYS_REG_MASK) | ((u8_t)w << 5);
    }

    J_A(AI,NODISC) inline constexpr bool is_gpr() const noexcept
    { return !(value & 16); }
    J_A(AI,NODISC,HIDDEN,ND) inline constexpr bool is_fp() const noexcept
    { return value & 16; }

    J_A(AI,NODISC) inline constexpr bool is(phys_reg p) const noexcept {
      return (value & PHYS_REG_MASK) == p.index;
    }
    J_A(AI,NODISC,ND,HIDDEN) inline constexpr mem_width width() const noexcept {
      return (mem_width)(value >> 5);
    }
  };

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg make_reg(u8_t idx, mem_width w) noexcept {
    return {(u8_t)(idx + ((u8_t)w << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg8(phys_reg r) noexcept {
    return {(u8_t)(r.index | ((u8_t)mem_width::byte << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg16(phys_reg r) noexcept {
    return {(u8_t)(r.index | ((u8_t)mem_width::word << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg32(phys_reg r) noexcept {
    return {(u8_t)(r.index | ((u8_t)mem_width::dword << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg64(phys_reg r) noexcept {
    return {(u8_t)(r.index | ((u8_t)mem_width::qword << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg8(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::byte << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg16(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::word << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg32(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::dword << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg64(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::qword << 5))};
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg128(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::vec128 << 5))};
  }
  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg256(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::vec256 << 5))};
  }
  J_A(AI,NODISC,ND,HIDDEN) inline constexpr reg reg512(u8_t index) noexcept {
    return {(u8_t)(index | ((u8_t)mem_width::vec512 << 5))};
  }

}
