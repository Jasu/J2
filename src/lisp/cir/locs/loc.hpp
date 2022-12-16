#pragma once

#include "lisp/assembly/registers.hpp"

namespace j::lisp::cir {
  enum class loc_type : u8_t {
    none = 0U,
    gpr,
    fp_reg,
    spill_slot,
  };

  struct loc final {
    J_BOILERPLATE(loc, CTOR_CE_ND, EQ)

    u16_t index = U16_MAX;

    J_A(AI,NODISC,ND) inline constexpr static loc from_phys_reg(assembly::phys_reg reg) noexcept {
      return loc(reg.is_fp() ? loc_type::fp_reg : loc_type::gpr, reg.index);
    }

    J_A(AI,NODISC) static inline loc make_spill_slot(u32_t index) noexcept {
      return loc(loc_type::spill_slot, index);
    }

    J_A(AI,NODISC) inline constexpr explicit operator bool() const noexcept
    { return index != U16_MAX; }

    J_A(AI,NODISC) inline constexpr bool operator!() const noexcept
    { return index == U16_MAX; }

    J_A(AI,NODISC) inline constexpr bool empty() const noexcept
    { return index == U16_MAX; }

    J_A(AI,NODISC) inline constexpr bool is_spill_slot() const noexcept
    { return (u16_t)(index + 1U) > 32; }

    J_A(AI,NODISC) inline constexpr bool is_gpr() const noexcept
    { return index < 16; }

    J_A(AI,NODISC) inline constexpr bool is_fp() const noexcept
    { return index >= 16 && index < 32; }

    J_A(AI,NODISC) inline constexpr bool is_reg() const noexcept
    { return index < 32; }

    [[nodiscard]] inline u16_t spill_slot_index() const noexcept {
      J_ASSERT(index != U16_MAX && index >= 32);
      return index - 32;
    }

    [[nodiscard]] inline assembly::phys_reg as_phys_reg() const noexcept {
      return {(u8_t)index};
    }

    [[nodiscard]] inline assembly::reg as_gpr64() const noexcept {
      return assembly::reg64(index);
    }

    [[nodiscard]] inline assembly::reg as_gpr32() const noexcept {
      return assembly::reg32(index);
    }

    [[nodiscard]] inline assembly::reg as_gpr16() const noexcept {
      return assembly::reg16(index);
    }
    [[nodiscard]] inline assembly::reg as_gpr8() const noexcept {
      return assembly::reg8(index);
    }

    [[nodiscard]] inline assembly::reg as_fp_reg() const noexcept {
      J_ASSERT(index >= 16);
      return assembly::reg128(index);
    }

    J_A(AI,NODISC) inline constexpr loc_type type() const noexcept {
      return index < 32 ? (index < 16 ? loc_type::gpr : loc_type::fp_reg)
        : index == U16_MAX ? loc_type::none : loc_type::spill_slot;
    }

    J_A(AI,NODISC) inline static constexpr loc from_raw_u16(u16_t index) noexcept {
      return loc(index);
     }
  private:
    J_A(AI) inline constexpr explicit loc(u16_t index) noexcept
      : index(index)
    { }

    J_A(AI) inline constexpr loc(loc_type type, u16_t index) noexcept
      : index(index + (type == loc_type::spill_slot ? 32 : 0))
    { }
  };
}
