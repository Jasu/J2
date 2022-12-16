#pragma once

#include "lisp/cir/locs/loc_mask.hpp"

namespace j::lisp::cir::inline locs {
  struct loc_specifier final {
    u16_t mask = U16_MAX;
    u16_t hint = 0U;

    const static loc_specifier any J_A(ND);
    const static loc_specifier none J_A(ND);

    J_A(AI,ND) inline constexpr loc_specifier() noexcept = default;

    J_A(AI,ND) inline constexpr explicit loc_specifier(loc loc) noexcept
      : mask(1U << (loc.index & 0xF))
    { J_ASSERT(loc.is_reg()); }

    J_A(AI,ND) inline constexpr loc_specifier(loc_mask mask) noexcept
      : mask(mask.mask >> 16 | mask.mask)
    { }

    J_A(AI) inline constexpr loc_specifier(loc_mask mask, loc_mask hint) noexcept
      : mask(mask.mask >> 16 | mask.mask),
        hint((hint.mask >> 16 | hint.mask) & this->mask)
    { }

    J_A(AI,ND) inline constexpr loc_specifier(u16_t mask, u16_t hint) noexcept
      : mask(mask),
        hint(hint & mask)
    { }

    J_A(AI,NODISC) inline bool empty() const noexcept { return !mask; }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return (bool)mask; }
    J_A(AI,NODISC) inline bool operator!() const noexcept { return !mask; }

    J_A(AI,NODISC) inline bool operator==(const loc_specifier &) const noexcept = default;

    constexpr inline static u16_t combine_hints(u16_t mask_a, u16_t mask_b, u16_t a, u16_t b) noexcept {
      u16_t mask = mask_a & mask_b;
      a &= mask;
      b &= mask;
      u16_t hint = a & b;
      if (hint && hint != mask) {
        return hint;
      }
      hint = a | b;
      if (hint && hint != mask) {
        return hint;
      }
      if (mask_a != mask) {
        return mask_a;
      }
      if (mask_b != mask) {
        return mask_b;
      }
      return 0U;
    }

    [[nodiscard]] inline constexpr loc_specifier operator&(const loc_specifier & rhs) const noexcept {
      return loc_specifier(mask & rhs.mask, combine_hints(mask, rhs.mask, hint, rhs.hint));
    }

    inline loc_specifier & operator&=(const loc_specifier & rhs) noexcept {
      hint = combine_hints(mask, rhs.mask, hint, rhs.hint);
      mask &= rhs.mask;
      return *this;
    }

    inline loc_specifier & operator&=(loc_mask rhs) noexcept {
      u16_t new_mask = (rhs.mask >> 16) | rhs.mask;
      hint = combine_hints(mask, new_mask, hint, 0U);
      return *this;
    }

    inline void remove(loc_mask m) noexcept {
      mask &= ~((m.mask >> 16) | m.mask);
      hint &= mask;
    }

    [[nodiscard]] inline loc_specifier without(loc_mask m) const noexcept {
      return loc_specifier(
        mask & ~((m.mask >> 16) | m.mask),
        hint);
    }

    [[nodiscard]] inline loc_specifier without(loc_specifier m) const noexcept {
      return loc_specifier(mask & ~m.mask, combine_hints(mask, ~m.mask, hint, ~m.hint));
    }

    [[nodiscard]] loc_specifier as_hint() const noexcept {
      return loc_specifier(0xFFFFU, hint ? hint : mask);
    }

    [[nodiscard]] inline loc_specifier with(loc_mask m) const noexcept {
      return loc_specifier(m.mask | (m.mask >> 16), hint);
    }

    [[nodiscard]] inline loc_specifier with_hint(loc l) const noexcept {
      return loc_specifier(mask, combine_hints(mask, mask, hint, 1U << (l.index & 15)));
    }

    [[nodiscard]] inline loc_specifier with_hint(loc_mask m) const noexcept {
      return loc_specifier(mask, combine_hints(mask, mask, hint, m.mask | (m.mask >> 16)));
    }

    [[nodiscard]] inline loc_specifier with_hint(loc_specifier m) const noexcept {
      return loc_specifier(mask, combine_hints(mask, m.mask, hint, m.hint));
    }

    [[nodiscard]] inline bool contains(loc reg) const noexcept {
      return reg.is_reg() && (mask & (1U << (reg.index & 15)));
    }

    [[nodiscard]] inline loc_mask get_mask(bool is_fp) const noexcept {
      return loc_mask(is_fp ? mask << 16 : mask);
    }
    [[nodiscard]] inline loc_mask get_hint(bool is_fp) const noexcept {
      return loc_mask(is_fp ? hint << 16 : hint);
    }
    inline void set_mask(loc_mask m) noexcept {
      mask = (m.mask >> 16) | m.mask;
      hint &= mask;
    }

    inline void set_hint(loc_mask m) noexcept {
      hint = (m.mask >> 16) | m.mask;
      hint &= mask;
    }
  };

  [[nodiscard]] constexpr inline loc_specifier operator&(const loc_specifier & lhs, const loc_mask & rhs) noexcept {
    return loc_specifier(lhs.mask & (rhs.mask >> 16 | rhs.mask), lhs.hint);
  }

  [[nodiscard]] constexpr inline loc_specifier operator&(const loc_mask & lhs, const loc_specifier & rhs) noexcept {
    return loc_specifier(rhs.mask & (lhs.mask >> 16 | lhs.mask), rhs.hint);
  }

  J_A(ND) constexpr inline const loc_specifier loc_specifier::any(0xFFFF, 0);
  J_A(ND) constexpr inline const loc_specifier loc_specifier::none(0, 0);
}
