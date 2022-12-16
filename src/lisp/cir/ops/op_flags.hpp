#pragma once

#include "lisp/cir/locs/loc_mask.hpp"

namespace j::lisp::cir::inline ops {
  enum class alias_mode : u8_t {
    disallow = 0U,
    allow = 1U,
    prefer = 3U,
  };

  enum class type_mode : u8_t {
    none = 0U,
    gpr = 1U,
    fp = 2U,
    any = 3U,
  };

  struct alias_flags final {
    u16_t alias_data = 0U;

    template<typename... Ts>
    constexpr explicit alias_flags(u16_t alias_data, Ts ...aliases) noexcept
      : alias_data((alias_data | ... | aliases))
    { }

    constexpr alias_mode input_alias(u8_t index) const noexcept {
      auto mask = 3U << (min(index + 1, 7) * 2);
      return (alias_mode)(alias_data & mask);
    }

    constexpr alias_mode result_alias() const noexcept {
      return (alias_mode)(alias_data & 0b11);
    }

    constexpr alias_flags operator|(const alias_flags & rhs) const noexcept {
      return alias_flags(alias_data | rhs.alias_data);
    }

    constexpr alias_flags with_result(alias_mode alias) const noexcept{
      return alias_flags((alias_data & 0xFFFD) | (u16_t)alias);
    }
    constexpr alias_flags not_result() const noexcept {
      return alias_flags(alias_data & ~0b0011U);
    }
    constexpr alias_flags not_arg(u8_t i) const noexcept {
      return alias_flags((alias_data & ~(0b1100U << (i * 2))));
    }
    constexpr alias_flags with_arg(u8_t i, alias_mode alias) const noexcept {
      return alias_flags(((alias_data & ~(0b1100U << (i * 2)))) | ((u16_t)alias << (i * 2 + 2)));
    }
    constexpr alias_flags with_all(alias_mode alias) const noexcept {
      return alias_flags(0x5555 * (u16_t)alias);
    }
    constexpr alias_flags with_all_args(alias_mode alias) const noexcept {
      return alias_flags((0x5554 * (u16_t)alias) | (0x3 & alias_data));
    }
  };

  constexpr inline alias_flags alias_any{0x5555U};
  constexpr inline alias_flags alias_any_arg{0x5554U};
  constexpr inline alias_flags alias_result{0x1};
  constexpr inline alias_flags alias_prefer_result{0x3};
  constexpr inline alias_flags alias_allow_arg0{0x1 << 2U};
  constexpr inline alias_flags alias_allow_arg1{0x1 << 4U};
  constexpr inline alias_flags alias_allow_arg2{0x1 << 6U};
  constexpr inline alias_flags alias_prefer_arg0{0x3 << 2U};
  constexpr inline alias_flags alias_prefer_arg1{0x3 << 4U};
  constexpr inline alias_flags alias_prefer_arg2{0x3 << 6U};
}
