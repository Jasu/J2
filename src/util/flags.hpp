#pragma once

#include "bits/bitops.hpp"

namespace j::util {
  template<typename Enum, typename Value>
  struct flag_iterator final {
    J_A(AI,NE,HIDDEN,NODISC) inline Enum operator*() const noexcept {
      return (Enum)bits::ctz(m_mask);
    }

    inline flag_iterator & operator++() noexcept {
      m_mask &= m_mask - 1;
      return *this;
    }

    J_A(AI,NE,HIDDEN) inline flag_iterator operator++(int) noexcept {
      Value v = m_mask;
      m_mask &= m_mask - 1;
      return {v};
    }

    J_A(AI,NE,HIDDEN,NODISC) inline bool operator==(const flag_iterator & rhs) const noexcept = default;

    Value m_mask = 0U;
  };

  template<typename Enum, typename Value>
  struct flags {
    using enum_t J_NO_DEBUG_TYPE = Enum;

    J_A(AI,ND,NE,HIDDEN) inline constexpr flags() noexcept = default;

    J_A(AI,ND,NE,HIDDEN) inline constexpr flags(Enum v) noexcept
      : value((Value)1 << (int)v)
    { }

    template<typename... Enums>
    J_A(AI,ND,NE,HIDDEN) inline constexpr flags(Enum v, Enums... vs) noexcept
      : value((((Value)1 << (int)v) | ... | ((Value)1 << (int)vs)))
    { }

    J_A(AI,ND,NE,HIDDEN) inline explicit constexpr flags(Value v) noexcept : value(v) { }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline flags operator&(flags rhs) const noexcept
    { return flags(value & rhs.value); }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline constexpr flags with(flags rhs) const noexcept
    { return flags(value | rhs.value); }
    J_A(AI,ND,NE,NODISC,HIDDEN) inline constexpr flags with(Enum rhs) const noexcept
    { return flags(value | ((Value)1 << ((int)rhs))); }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline constexpr flags without(flags rhs) const noexcept
    { return flags(value & ~rhs.value); }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline constexpr flags operator|(flags rhs) const noexcept { return flags(value | rhs.value); }
    J_A(AI,ND,NE,NODISC,HIDDEN) inline constexpr flags operator|(Enum rhs) const noexcept { return flags(value | ((Value)1 << (int)rhs)); }
    J_A(AI,ND,NE,NODISC,HIDDEN) inline flags operator-(flags rhs) const noexcept { return without(rhs); }

    J_A(AI,ND,NE,HIDDEN) inline flags & operator|=(flags rhs) noexcept {
      value |= rhs.value;
      return *this;
    }

    J_A(AI,ND,NE,HIDDEN) inline flags & operator&=(flags rhs) noexcept {
      value &= rhs.value;
      return *this;
    }

    J_A(AI,ND,NE,HIDDEN) inline flags operator-=(flags rhs) noexcept {
      value &= ~rhs.value;
      return *this;
    }

    J_A(ND,NODISC) auto begin() const noexcept;

    J_A(ND,NODISC) auto end() const noexcept;

    J_A(ND) void clear(Enum v) noexcept {
      value &= ~((Value)1 << (int)v);
    }
    J_A(AI,ND,HIDDEN,NE) inline void set(Enum v) noexcept {
      value |= (Value)1 << (int)v;
    }
    J_A(AI,ND,HIDDEN,NE) inline void set(Enum v, bool to) noexcept {
      Value mask = (Value)1 << (int)v;
      value = (value & ~mask) | (to ? mask : (Value)0);
    }

    J_A(AI,NODISC,ND,HIDDEN,NE) inline bool has(Enum v) const noexcept {
      return value & (1UL << static_cast<u8_t>(v));
    }

    J_A(AI,NODISC,ND,HIDDEN,NE) inline bool operator==(const flags & rhs) const noexcept = default;
    J_A(AI,NODISC,ND,HIDDEN,NE) inline bool operator==(Enum v) const noexcept {
      return value == 1U << (int)v;
    }

    J_A(ND,NODISC) bool is_subset_of(const flags & rhs) const noexcept
    { return (value & rhs.value) == value; }

    J_A(ND,NODISC) bool is_superset_of(const flags & rhs) const noexcept
    { return (value & rhs.value) == rhs.value; }

    J_A(ND,NODISC) bool is_disjoint_with(const flags & rhs) const noexcept
    { return !(value & rhs.value); }

    J_A(ND,NODISC) bool intersects_with(const flags & rhs) const noexcept
    { return value & rhs.value; }


    J_A(AI,NODISC,ND,NE,HIDDEN) inline bool empty() const noexcept { return !value; }
    J_A(AI,NODISC,ND,NE,HIDDEN) inline operator bool() const noexcept        { return value; }
    J_A(AI,NODISC,ND,NE,HIDDEN) inline bool operator!() const noexcept       { return !value; }

    J_A(ND,NODISC) Enum first_value() const noexcept {
      return (Enum)bits::ctz(value);
    }
    J_A(ND,NODISC) bool has_single_value() const noexcept {
      return !(value & (value - 1));
    }

    J_A(NODISC) u8_t size() const noexcept {
      return bits::popcount(value);
    }

    J_A(NODISC) Enum only_value() const noexcept {
      J_ASSERT(!(value & (value - (Value)1)), "Multiple values");
      return (Enum)bits::ctz(value);
    }

    Value value = 0;
  };

  template<typename E, typename V>
  J_A(ND,NODISC) auto flags<E, V>::begin() const noexcept {
    return flag_iterator<E, V>{ value };
  }

  template<typename E, typename V>
  J_A(ND,NODISC) auto flags<E, V>::end() const noexcept {
    return flag_iterator<E, V>{ (V)0 };
  }
}

#define J_DECLARE_EXTERN_FLAGS(ENUM, VALUE) extern template struct j::util::flags<ENUM, VALUE>
#define J_DEFINE_EXTERN_FLAGS(ENUM, VALUE) template struct j::util::flags<ENUM, VALUE>

#define J_FLAG_OPERATORS(ENUM, VALUE)                                                             \
  J_A(AI,ND,HIDDEN,NE,NODISC) constexpr inline ::j::util::flags<ENUM, VALUE> operator|(ENUM lhs, const ::j::util::flags<ENUM, VALUE> & rhs) noexcept { \
    return rhs.with(lhs);                                                                         \
  }                                                                                               \
  J_A(AI,ND,HIDDEN,NE,NODISC) constexpr inline ::j::util::flags<ENUM, VALUE> operator|(ENUM lhs, ENUM rhs) noexcept {                                  \
    return {rhs, lhs };                                                 \
  }
