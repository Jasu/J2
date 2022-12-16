#pragma once

#include "hzd/utility.hpp"

namespace j::util {
  /// Three-valued logic "boolean", intended for configuration settings.
  ///
  /// Values can be true, false, or indeterminate (unknown). Indeterminate cases are handled
  /// so that the result can never be wrong - i.e. logical connectives result in indeterminate
  /// values if changing any indeterminate value in the expression could change the result.
  ///
  /// Currently, negation is not implemented, as it is not needed for simply handling
  /// config value overriding. Conveniently, omitting negation weakens the system enough
  /// to avoid having to accept or attempt to reject that both `x ∧ ¬x` and `x ∨ ¬x` would
  /// be indeterminate.
  ///
  /// true, respectively. As `tristate` is intended for configuration defaults,
  /// this doesn't probably matter though.
  struct tristate final {
    J_BOILERPLATE(tristate, CTOR_CE)

    J_A(AI,ND) inline constexpr tristate(null_t) noexcept { }

    J_A(AI,ND) inline constexpr tristate(bool value) noexcept
      : m_value(value ? 1 : -1)
    { }

    /// Logical or (adheres to Kleene's logic, i.e. strong indeterminancy.)
    ///
    /// ```
    ///     F   I   T
    ///   +---+---+---+
    /// F | F | I | T |
    ///   +---+---+---+
    /// I | I | I | T |
    ///   +---+---+---+
    /// T | T | T | T |
    ///   +---+---+---+
    /// ```
    /// Having either operand as true makes the result true.
    J_A(AI,ND,NODISC) inline constexpr tristate operator|(const tristate & rhs) const noexcept {
      return tristate(m_value > rhs.m_value ? m_value : rhs.m_value);
    }

    J_A(AI,ND,NODISC) inline friend constexpr tristate operator|(bool lhs, const tristate & rhs) noexcept {
      return lhs ? tristate(true) : rhs;
    }

    J_A(AI,ND,NODISC) inline friend constexpr tristate operator|(const tristate & lhs, bool rhs) noexcept {
      return rhs ? tristate(true) : lhs;
    }

    J_A(AI,ND) inline constexpr tristate & operator|=(const tristate & rhs) noexcept {
      m_value = m_value >= rhs.m_value ? m_value : rhs.m_value;
      return *this;
    }

    J_A(AI,ND) inline constexpr tristate & operator|=(bool rhs) noexcept {
      m_value = rhs ? 1 : m_value;
      return *this;
    }

    /// Logical and (adheres to Kleene's logic, i.e. strong indeterminancy.)
    ///
    /// ```
    ///     F   I   T
    ///   +---+---+---+
    /// F | F | F | F |
    ///   +---+---+---+
    /// I | F | I | I |
    ///   +---+---+---+
    /// T | F | I | T |
    ///   +---+---+---+
    /// ```
    /// Having either operand as false makes the result false. Having both true makes the
    /// result true, other values yield indeterminate values.
    J_A(AI,ND,NODISC) inline constexpr tristate operator&(const tristate & rhs) const noexcept {
      return tristate(m_value <= rhs.m_value ? m_value : rhs.m_value);
    }

    J_A(AI,ND) inline constexpr tristate & operator&=(const tristate & rhs) noexcept {
      m_value = m_value <= rhs.m_value ? m_value : rhs.m_value;
      return *this;
    }

    J_A(AI,ND) inline constexpr tristate & operator&=(bool rhs) noexcept {
      m_value = rhs ? m_value : -1;
      return *this;
    }

    J_A(AI,ND,NODISC) inline friend constexpr tristate operator&(bool lhs, const tristate & rhs) noexcept {
      return lhs ? rhs : tristate(false);
    }

    J_A(AI,ND,NODISC) inline friend constexpr tristate operator&(const tristate & lhs, bool rhs) noexcept {
      return rhs ? lhs : tristate(false);
    }

    /// Apply a default to the value. If `this` is indeterminate, it will be replaced with `rhs`.
    ///
    /// This does not map neatly to any logical connectives.
    J_A(AI,ND,NODISC) inline constexpr bool operator/(bool rhs) const noexcept {
      return m_value ? (m_value > 0) : rhs;
    }

    /// Apply a default to the value. If `this` is indeterminate, it will be replaced with `rhs`.
    ///
    /// This does not map neatly to any logical connectives.
    J_A(AI,ND,NODISC) inline constexpr tristate operator/(const tristate & rhs) const noexcept {
      return tristate(m_value ? m_value : rhs.m_value);
    }

    /// Apply a default to the value. If `this` is indeterminate, it will be replaced with `rhs`.
    ///
    /// This does not map neatly to any logical connectives.
    J_A(AI,ND) inline constexpr tristate & operator/=(const tristate & rhs) noexcept {
      m_value = m_value ? m_value : rhs.m_value;
      return *this;
    }

    /// Apply a default to the value. If `this` is indeterminate, it will be replaced with `rhs`.
    ///
    /// This does not map neatly to any logical connectives.
    J_A(AI,ND) inline constexpr tristate & operator/=(bool rhs) noexcept {
      m_value = m_value ? m_value : (rhs ? 1 : -1);
      return *this;
    }

    J_A(AI,ND,NODISC) inline constexpr bool operator==(const tristate &) const noexcept = default;

    J_A(AI,ND,NODISC) inline constexpr bool is_true() const noexcept { return m_value > 0; }
    J_A(AI,ND,NODISC) inline constexpr bool is_false() const noexcept { return m_value < 0; }
    J_A(AI,ND,NODISC) inline constexpr bool has_value() const noexcept { return m_value; }
    J_A(AI,ND,NODISC) inline constexpr bool is_indeterminate() const noexcept { return !m_value; }

    J_A(AI,ND,NODISC) inline constexpr i8_t value() const noexcept { return m_value; }

  private:
    J_A(AI,ND) inline constexpr explicit tristate(i8_t value) noexcept
      : m_value(value)
    { }

    i8_t m_value = 0;
  };
}
