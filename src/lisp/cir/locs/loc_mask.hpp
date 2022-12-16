#pragma once

#include "exceptions/assert_lite.hpp"
#include "lisp/cir/locs/loc.hpp"
#include "bits/bitops.hpp"

namespace j::lisp::cir::inline locs {
  /// A glorified `unsigned int` for storing register bitmasks, and being explicit about
  /// the semantics.
  struct loc_mask final {
    u32_t mask = 0U;

    J_BOILERPLATE(loc_mask, CTOR_CE)

    constexpr loc_mask(const loc & loc) noexcept
      : mask (1U << loc.index)
    {
      J_ASSUME(mask);
    }

    J_ALWAYS_INLINE constexpr bool operator==(const loc_mask &) const noexcept = default;

    [[nodiscard]] inline constexpr loc_mask operator&(const loc_mask & rhs) const noexcept {
      return loc_mask(mask & rhs.mask);
    }

    inline constexpr loc_mask & operator&=(const loc_mask & rhs) noexcept {
      mask &= rhs.mask;
      return *this;
    }

    [[nodiscard]] inline constexpr loc_mask operator-(const loc_mask & rhs) const noexcept {
      return loc_mask(mask & ~rhs.mask);
    }

    inline constexpr loc_mask & operator-=(const loc_mask & rhs) noexcept {
      mask &= ~rhs.mask;
      return *this;
    }

    [[nodiscard]] inline constexpr loc_mask operator|(const loc_mask & rhs) const noexcept {
      return loc_mask(mask | rhs.mask);
    }

    inline constexpr loc_mask & operator|=(const loc_mask & rhs) noexcept {
      mask |= rhs.mask;
      return *this;
    }

    [[nodiscard]] inline constexpr loc_mask operator~() const noexcept {
      return loc_mask(~mask);
    }

    [[nodiscard]] inline constexpr loc_mask with(const loc_mask & rhs) const noexcept {
      return loc_mask(mask | ~rhs.mask);
    }

    [[nodiscard]] inline constexpr loc_mask without(const loc_mask & rhs) const noexcept {
      return loc_mask(mask & ~rhs.mask);
    }

    inline constexpr void remove(const loc_mask & rhs) noexcept {
      mask &= ~rhs.mask;
    }

    inline constexpr void add(const loc_mask & rhs) noexcept {
      mask |= rhs.mask;
    }

    [[nodiscard]] inline constexpr bool overlaps_with(const loc_mask & rhs) const noexcept {
      return mask & rhs.mask;
    }

    [[nodiscard]] inline constexpr bool contains(const loc & loc) const noexcept {
      J_ASSERT(loc.is_reg());
      return mask & (1U << loc.index);
    }

    [[nodiscard]] inline constexpr bool covers(const loc_mask & rhs) const noexcept {
      return (mask & rhs.mask) == rhs.mask;
    }

    [[nodiscard]] inline constexpr bool excludes(const loc_mask & rhs) const noexcept {
      return !(mask & rhs.mask);
    }

    [[nodiscard]] loc find_first_set() const noexcept {
      if (!mask) {
        return loc{};
      }
      u32_t index = bits::ctz(mask);
      J_ASSUME(index < 32);
      return loc::from_raw_u16(index);
    }

    loc find_and_clear_first_set() noexcept {
      loc result = find_first_set();
      mask = mask & (mask - 1U);
      return result;
    }

    [[nodiscard]] u8_t size() const noexcept {
      return bits::popcount(mask);
    }

    [[nodiscard]] constexpr bool is_fp() const noexcept {
      return 0xFFFF0000U & mask;
    }

    [[nodiscard]] constexpr bool is_gpr() const noexcept {
      return 0x0000FFFFU & mask;
    }

    [[nodiscard]] constexpr bool has_single_value() const noexcept {
      return !((mask - 1) & mask);
    }

    J_INLINE_GETTER constexpr bool empty() const noexcept { return !mask; }
    J_INLINE_GETTER constexpr explicit operator bool() const noexcept { return mask; }
    J_INLINE_GETTER constexpr bool operator!() const noexcept { return !mask; }

    const static loc_mask any;
    const static loc_mask none;
    const static loc_mask any_gpr;
    const static loc_mask any_fp;

    J_ALWAYS_INLINE explicit constexpr loc_mask(u32_t mask) noexcept
      : mask(mask)
    { }
  };

  constexpr loc_mask loc_mask::any(U32_MAX);
  constexpr loc_mask loc_mask::none(0U);
  constexpr loc_mask loc_mask::any_gpr((1U << 16) - 1);
  constexpr loc_mask loc_mask::any_fp (~((1U << 16) - 1));
}
