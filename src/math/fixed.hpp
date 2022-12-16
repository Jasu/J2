#pragma once

#include "hzd/integer_traits.hpp"
#include "hzd/concepts.hpp"

namespace j::math {
  namespace detail {
    /// Constructor tag indicating that the input number should be rounded down.
    struct truncate_tag_t {
      template<typename T, u8_t>
      J_NO_DEBUG inline constexpr static T rounding_bias = 0;
    };
    /// Constructor tag indicating that the input number should be rounded.
    struct round_tag_t {
      template<typename T, u8_t right_shift>
      J_NO_DEBUG inline constexpr static T rounding_bias = (T)1 << (right_shift - 1U);
    };
    /// Constructor tag indicating that the input number should be rounded up.
    struct ceil_tag_t {
      template<typename T, u8_t right_shift>
      J_NO_DEBUG inline constexpr static T rounding_bias = ((T)1 << right_shift) - (T)1;
    };
  }

  enum raw_tag_t { raw_tag };

  /// Constructor tag indicating that the input number should be rounded down.
  J_NO_DEBUG inline constexpr detail::truncate_tag_t truncate_tag{};

  /// Constructor tag indicating that the input number should be rounded.
  J_NO_DEBUG inline constexpr detail::round_tag_t round_tag{};

  /// Constructor tag indicating that the input number should be rounded up.
  J_NO_DEBUG inline constexpr detail::ceil_tag_t ceil_tag{};

  using default_rounding_mode_t J_NO_DEBUG_TYPE = detail::truncate_tag_t;
  inline constexpr default_rounding_mode_t default_rounding_mode_v{};

  template<typename Integer, u8_t FractionBits> class fixed;

  namespace detail {
    template<u32_t Size, bool IsSigned>
    struct J_TYPE_HIDDEN integer_type;

    template<> struct J_TYPE_HIDDEN integer_type<1, false>  { using type J_NO_DEBUG_TYPE = u8_t; };
    template<> struct J_TYPE_HIDDEN integer_type<1, true>   { using type J_NO_DEBUG_TYPE = i8_t; };
    template<> struct J_TYPE_HIDDEN integer_type<2, false>  { using type J_NO_DEBUG_TYPE = u16_t; };
    template<> struct J_TYPE_HIDDEN integer_type<2, true>   { using type J_NO_DEBUG_TYPE = i16_t; };
    template<> struct J_TYPE_HIDDEN integer_type<4, false>  { using type J_NO_DEBUG_TYPE = u32_t; };
    template<> struct J_TYPE_HIDDEN integer_type<4, true>   { using type J_NO_DEBUG_TYPE = i32_t; };
    template<> struct J_TYPE_HIDDEN integer_type<8, false>  { using type J_NO_DEBUG_TYPE = u64_t; };
    template<> struct J_TYPE_HIDDEN integer_type<8, true>   { using type J_NO_DEBUG_TYPE = i64_t; };

#ifdef __SIZEOF_INT128__
    template<> struct J_TYPE_HIDDEN integer_type<16, false> { using type J_NO_DEBUG_TYPE = u128_t; };
    template<> struct J_TYPE_HIDDEN integer_type<16, true>  { using type J_NO_DEBUG_TYPE = i128_t; };
#endif

    template<u32_t Size, bool IsSigned>
    using integer_type_t J_NO_DEBUG_TYPE = typename integer_type<Size, IsSigned>::type;

    template<typename TargetInteger, u8_t TargetFraction,
             typename SourceInteger, u8_t SourceFraction,
             typename RoundingMode = default_rounding_mode_t>
    J_A(AI,NODISC,ND,HIDDEN) inline static constexpr TargetInteger convert_to(SourceInteger source, const RoundingMode & = default_rounding_mode_v) noexcept {
      constexpr int left_shift = (int)TargetFraction - (int)SourceFraction;
      if constexpr (left_shift == 0) {
        return source;
      } else if constexpr (left_shift > 0) {
        return source * (TargetInteger(1) << left_shift);
      } else {
        return (source + RoundingMode::template rounding_bias<SourceInteger, -left_shift>) >> -left_shift;
      }
    }

    template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction>
    struct J_TYPE_HIDDEN promote_helper {
      J_NO_DEBUG inline constexpr static u32_t largest_size = sizeof(LhsInteger) > sizeof(RhsInteger) ? sizeof(LhsInteger) : sizeof(RhsInteger);
      J_NO_DEBUG inline constexpr static bool is_signed = is_signed_v<LhsInteger> || is_signed_v<RhsInteger>;
      J_NO_DEBUG inline constexpr static u8_t result_fraction = LhsFraction < RhsFraction ? RhsFraction : LhsFraction;
      using temp_integer_t J_NO_DEBUG_TYPE = typename integer_type<largest_size * 2, is_signed>::type;
      using result_t J_NO_DEBUG_TYPE = fixed<typename integer_type<largest_size, is_signed>::type, result_fraction>;

      template<typename Integer, u8_t Fraction>
      J_A(AI,ND,NODISC,HIDDEN) inline constexpr static temp_integer_t to_temp_integer(const fixed<Integer, Fraction> & input) noexcept {
        return (temp_integer_t)input.raw() << (result_fraction - Fraction);
      }
    };

    template<typename Integer, u8_t Fraction>
    struct J_TYPE_HIDDEN promote_helper<Integer, Fraction, Integer, Fraction> {
      J_NO_DEBUG inline constexpr static bool is_signed = is_signed_v<Integer>;
      J_NO_DEBUG inline constexpr static u8_t result_fraction = Fraction;
      using temp_integer_t J_NO_DEBUG_TYPE = next_larger_t<Integer>;
      using result_t J_NO_DEBUG_TYPE = fixed<Integer, Fraction>;

      J_A(AI,ND,NODISC,HIDDEN) inline constexpr static temp_integer_t to_temp_integer(const fixed<Integer, Fraction> & input) noexcept {
        return input.raw();
      }
    };
  }

  template<typename Integer, u8_t FractionBits>
  class fixed {
  public:
    using inner_type = Integer;
    inline constexpr static const Integer one = Integer(1) << FractionBits;
    inline constexpr static const Integer fraction_mask = one - Integer(1);
    inline constexpr static const Integer highest_raw = j::max_value_v<Integer>;
    inline constexpr static const Integer lowest_raw = j::min_value_v<Integer>;
    inline constexpr static const Integer highest_int = highest_raw >> FractionBits;
    inline constexpr static const Integer lowest_int = lowest_raw >> FractionBits;
    inline constexpr static const bool is_signed = j::is_signed_v<Integer>;

    Integer m_value;
  public:
    /// Construct an uninitialized fixed-point number.
    J_A(AI,ND,HIDDEN) inline fixed() noexcept = default;

    J_A(AI,ND,HIDDEN) inline fixed(const fixed & rhs) noexcept = default;
    J_A(AI,ND,HIDDEN) inline fixed & operator=(const fixed & rhs) noexcept = default;

    template<typename SourceInteger, u8_t SourceFraction, typename RoundingMode = default_rounding_mode_t>
    explicit constexpr fixed(const fixed<SourceInteger, SourceFraction> & rhs, const RoundingMode & rounding_mode = default_rounding_mode_v) noexcept;

    /// Initialize the fixed-point number from an integer or a floating-point value.
    template<FloatingPoint Other>
    J_A(AI,ND,HIDDEN) inline fixed(Other value) noexcept : m_value(value * one) { }

    template<Integral Other>
    J_A(AI,ND,HIDDEN) constexpr inline fixed(Other value) noexcept : m_value(value << FractionBits) { }

    /// Initialize the fixed-point number from a raw value.
    J_A(AI,ND,HIDDEN) inline constexpr fixed(Integer value, raw_tag_t) noexcept
      : m_value(value)
    { }

    /// Initialize the fixed-point number from an integral and a fractional part.
    explicit constexpr fixed(Integer integer, Integer fraction) noexcept;

    /// Get floor of the fixed-point number as an integer.
    J_A(AI,ND,NODISC,HIDDEN) inline constexpr Integer floor() const noexcept {
      return m_value >> FractionBits;
    }

    /// Get ceiling of the fixed-point number as an integer.
    J_A(AI,ND,NODISC,HIDDEN) inline constexpr Integer ceil() const noexcept {
      return (m_value + fraction_mask) >> FractionBits;
    }

    /// Get rounded fixed-point number as an integer.
    constexpr Integer round() const noexcept;

    J_A(NODISC,AI,ND,HIDDEN) inline constexpr float as_float() const noexcept {
      return (float)m_value / one;
    }

    J_A(NODISC,AI,ND,HIDDEN) inline constexpr double as_double() const noexcept {
      return (double)m_value / one;
    }

    /// Get the raw value of the fixed-point number as integer.
    J_A(AI,ND,NODISC,HIDDEN) inline constexpr Integer raw() const noexcept {
      return m_value;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline constexpr Integer fraction() const noexcept {
      return m_value & fraction_mask;
    }

    template<Integral Other>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator+=(Other rhs) noexcept {
      m_value += (Integer)rhs << FractionBits;
      return *this;
    }

    template<FloatingPoint Other>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator+=(Other rhs) noexcept {
      m_value += rhs * one;
      return *this;
    }

    template<Integral Other>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator-=(Other rhs) noexcept {
      m_value -= (Integer)rhs << FractionBits;
      return *this;
    }

    template<FloatingPoint Other>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator-=(Other rhs) noexcept {
      m_value -= rhs * one;
      return *this;
    }

    template<Arithmetic OtherScalar>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator*=(OtherScalar rhs) noexcept {
      m_value *= rhs;
      return *this;
    }

    template<Arithmetic OtherScalar>
    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator/=(OtherScalar rhs) noexcept {
      m_value /= rhs;
      return *this;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr fixed operator-(fixed rhs) const noexcept {
      return {m_value - rhs.m_value, raw_tag};
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr fixed operator+(fixed rhs) const noexcept {
      return {m_value + rhs.m_value, raw_tag};
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr bool operator==(fixed rhs) const noexcept {
      return m_value == rhs.m_value;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr bool operator<(fixed rhs) const noexcept {
      return m_value < rhs.m_value;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr bool operator<=(fixed rhs) const noexcept {
      return m_value <= rhs.m_value;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr bool operator>=(fixed rhs) const noexcept {
      return m_value >= rhs.m_value;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline constexpr bool operator>(fixed rhs) const noexcept {
      return m_value > rhs.m_value;
    }

    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator+=(fixed rhs) noexcept {
      m_value += rhs.m_value;
      return *this;
    }

    J_A(AI,ND,HIDDEN) inline constexpr fixed & operator-=(fixed rhs) noexcept {
      m_value -= rhs.m_value;
      return *this;
    }

    template<typename OtherInteger, u8_t OtherFraction>
    constexpr fixed & operator+=(const fixed<OtherInteger, OtherFraction> & rhs) noexcept;

    template<typename OtherInteger, u8_t OtherFraction>
    constexpr fixed & operator-=(const fixed<OtherInteger, OtherFraction> & rhs) noexcept;

    template<typename OtherInteger, u8_t OtherBits>
    constexpr fixed & operator*=(const fixed<OtherInteger, OtherBits> & rhs) noexcept;

    template<typename OtherInteger, u8_t OtherBits>
    constexpr fixed & operator/=(const fixed<OtherInteger, OtherBits> & rhs) noexcept;
  };

  template<typename Integer, u8_t FractionBits>
  template<typename SourceInteger, u8_t SourceFraction, typename RoundingMode>
  constexpr fixed<Integer, FractionBits>::fixed(const fixed<SourceInteger, SourceFraction> & rhs, const RoundingMode & rounding_mode) noexcept
    : m_value(detail::convert_to<Integer, FractionBits, SourceInteger, SourceFraction, RoundingMode>(rhs.raw(), rounding_mode))
  {
  }

  template<typename Integer, u8_t FractionBits>
  constexpr fixed<Integer, FractionBits>::fixed(Integer integer, Integer fraction) noexcept
    : m_value((integer << FractionBits) | (fraction & fraction_mask))
  {
  }

  template<typename Integer, u8_t FractionBits>
  constexpr Integer fixed<Integer, FractionBits>::round() const noexcept {
    return (m_value + (one >> 1)) >> FractionBits;
  }

  template<typename Integer, u8_t FractionBits>
  template<typename OtherInteger, u8_t OtherFraction>
  constexpr fixed<Integer, FractionBits> & fixed<Integer, FractionBits>::operator+=(const fixed<OtherInteger, OtherFraction> & rhs) noexcept {
    m_value += detail::convert_to<Integer, FractionBits, OtherInteger, OtherFraction>(rhs.raw());
    return *this;
  }

  template<typename Integer, u8_t FractionBits>
  template<typename OtherInteger, u8_t OtherFraction>
  constexpr fixed<Integer, FractionBits> & fixed<Integer, FractionBits>::operator-=(const fixed<OtherInteger, OtherFraction> & rhs) noexcept {
    m_value -= detail::convert_to<Integer, FractionBits, OtherInteger, OtherFraction>(rhs.raw());
    return *this;
  }

  template<typename Integer, u8_t FractionBits>
  template<typename OtherInteger, u8_t OtherBits>
  constexpr fixed<Integer, FractionBits> & fixed<Integer, FractionBits>::operator*=(const fixed<OtherInteger, OtherBits> & rhs) noexcept {
    using temp_t = next_larger_t<Integer>;
    auto result = detail::convert_to<temp_t, FractionBits + 1>(rhs.raw()) * (temp_t)m_value + (temp_t)one;
    m_value = result >> (FractionBits + 1);
    return *this;
  }

template<typename Integer, u8_t FractionBits>
template<typename OtherInteger, u8_t OtherBits>
constexpr fixed<Integer, FractionBits> & fixed<Integer, FractionBits>::operator/=(const fixed<OtherInteger, OtherBits> & rhs) noexcept {
  using temp_t = next_larger_t<Integer>;
  temp_t result = (temp_t)m_value * (temp_t(1) << temp_t(OtherBits + 1));
  m_value = (result / (temp_t)rhs.raw() + 1) >> 1;
  return *this;
}

template<typename Fixed>
J_NO_DEBUG inline constexpr Fixed highest_v{Fixed::highest_raw, raw_tag};

template<typename Fixed>
J_NO_DEBUG inline constexpr Fixed lowest_v{Fixed::lowest_raw, raw_tag};

template<typename LhsInteger, u8_t LhsFraction,
          typename RhsInteger, u8_t RhsFraction,
          typename Helper = detail::promote_helper<LhsInteger, LhsFraction, RhsInteger, RhsFraction>>
constexpr typename Helper::result_t operator+(const fixed<LhsInteger, LhsFraction> & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
  return typename Helper::result_t(Helper::to_temp_integer(lhs) + Helper::to_temp_integer(rhs), raw_tag);
}

  template<typename LhsInteger, u8_t LhsFraction, Integral Rhs>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr fixed<LhsInteger, LhsFraction> operator+(fixed<LhsInteger, LhsFraction> lhs, Rhs rhs) noexcept {
    return {lhs.raw() + ((LhsInteger)rhs << LhsFraction), raw_tag};
  }

  template<typename RhsInteger, u8_t RhsFraction, Integral Lhs>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr fixed<RhsInteger, RhsFraction> operator+(Lhs lhs, fixed<RhsInteger, RhsFraction> rhs) noexcept {
    return {rhs.raw() + ((RhsInteger)lhs << RhsFraction), raw_tag};
  }

  template<typename LhsInteger, u8_t LhsFraction, FloatingPoint Rhs>
  constexpr fixed<LhsInteger, LhsFraction> operator+(fixed<LhsInteger, LhsFraction> lhs, Rhs rhs) noexcept {
    return {lhs.raw() + (LhsInteger)(fixed<LhsInteger, LhsFraction>::one * rhs), raw_tag};
  }

  template<typename RhsInteger, u8_t RhsFraction, FloatingPoint Lhs>
  constexpr fixed<RhsInteger, RhsFraction> operator+(Lhs lhs, fixed<RhsInteger, RhsFraction> rhs) noexcept {
    return {rhs.raw() + (RhsInteger)(fixed<RhsInteger, RhsFraction>::one * lhs), raw_tag};
  }

  template<typename LhsInteger, u8_t LhsFraction, Arithmetic Rhs>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr fixed<LhsInteger, LhsFraction> operator*(const fixed<LhsInteger, LhsFraction> & lhs, Rhs rhs) noexcept {
    return fixed<LhsInteger, LhsFraction>(lhs.raw() * rhs, raw_tag);
  }

  template<Arithmetic Lhs, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) constexpr fixed<RhsInteger, RhsFraction> operator*(Lhs lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return fixed<RhsInteger, RhsFraction>(rhs.raw() * lhs, raw_tag);
  }

  template<typename LhsInteger, u8_t LhsFraction, FloatingPoint Rhs>
  constexpr fixed<LhsInteger, LhsFraction> operator-(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {
    return fixed<LhsInteger, LhsFraction>(lhs.raw() - (LhsInteger)(rhs * fixed<LhsInteger, LhsFraction>::one), raw_tag);
  }

  template<typename LhsInteger, u8_t LhsFraction, Integral Rhs>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr fixed<LhsInteger, LhsFraction> operator-(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {
    return fixed<LhsInteger, LhsFraction>(lhs.raw() - ((LhsInteger)rhs << LhsFraction), raw_tag);
  }

  template<Arithmetic Lhs, typename RhsInteger, u8_t RhsFraction>
  constexpr fixed<RhsInteger, RhsFraction> operator-(Lhs lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return fixed<RhsInteger, RhsFraction>(lhs * fixed<RhsInteger, RhsFraction>::one - rhs.raw(), raw_tag);
  }

  template<typename LhsInteger, u8_t LhsFraction, Arithmetic Rhs>
  constexpr fixed<LhsInteger, LhsFraction> operator/(const fixed<LhsInteger, LhsFraction> & a, const Rhs & b) noexcept {
    return fixed<LhsInteger, LhsFraction>(a.raw() / b, raw_tag);
  }

  template<typename RhsInteger, u8_t RhsFraction, Arithmetic Lhs>
  constexpr fixed<RhsInteger, RhsFraction> operator/(const Lhs & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return fixed<RhsInteger, RhsFraction>(lhs * (next_larger_t<Lhs>(1) << RhsFraction) / rhs.raw(), raw_tag);
  }

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction,
           typename Helper = detail::promote_helper<LhsInteger, LhsFraction, RhsInteger, RhsFraction>>
  constexpr typename Helper::result_t operator-(
    const fixed<LhsInteger, LhsFraction> & lhs,
    const fixed<RhsInteger, RhsFraction> & rhs) noexcept
  {
    return typename Helper::result_t(Helper::to_temp_integer(lhs) - Helper::to_temp_integer(rhs), raw_tag);
  }

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction,
           typename Helper = detail::promote_helper<LhsInteger, LhsFraction, RhsInteger, RhsFraction>>
  constexpr typename Helper::result_t operator*(
    const fixed<LhsInteger, LhsFraction> & lhs,
    const fixed<RhsInteger, RhsFraction> & rhs) noexcept
  {
    const auto temp = Helper::to_temp_integer(lhs) * Helper::to_temp_integer(rhs);
    return (temp + (1ULL << (Helper::result_fraction - 1ULL))) >> Helper::result_fraction;
  }

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction,
           typename Helper = detail::promote_helper<LhsInteger, LhsFraction, RhsInteger, RhsFraction>>
  constexpr typename Helper::result_t operator/(
    const fixed<LhsInteger, LhsFraction> & lhs,
    const fixed<RhsInteger, RhsFraction> & rhs) noexcept
  {
    using temp_t J_NO_DEBUG_TYPE = typename Helper::temp_integer_t;
    temp_t temp = Helper::to_temp_integer(lhs) * ((temp_t)1 << Helper::result_fraction);
    return typename Helper::result_t(temp / Helper::to_temp_integer(rhs), raw_tag);
  }

#define J_MATH_FIXED_RELOP(OP)                                                                               \
  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction>                     \
  J_INLINE_GETTER constexpr bool operator OP(const fixed<LhsInteger, LhsFraction> & lhs,                                     \
                                             const fixed<RhsInteger, RhsFraction> & rhs) noexcept { \
    using helper = detail::promote_helper<LhsInteger, LhsFraction, RhsInteger, RhsFraction>; \
    return helper::to_temp_integer(lhs) OP helper::to_temp_integer(rhs);                                     \
  }                                                                                                          \
                                                                                                             \
  template<typename LhsInteger, u8_t LhsFraction, Integral Rhs>                                              \
  J_INLINE_GETTER constexpr bool operator OP(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {         \
    return lhs.raw() OP (rhs * (next_larger_t<LhsInteger>(1) << LhsFraction));               \
  }                                                                                                          \
                                                                                                             \
  template<Integral Lhs, typename RhsInteger, u8_t RhsFraction>                                              \
  J_INLINE_GETTER constexpr bool operator OP(const Lhs & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {         \
    return (lhs * (next_larger_t<RhsInteger>(1) << RhsFraction)) OP rhs.raw();               \
  }

  J_MATH_FIXED_RELOP(==)
  J_MATH_FIXED_RELOP(<)

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>(const fixed<LhsInteger, LhsFraction> & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return operator<(rhs, lhs);
  }

  template<Integral Lhs, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>(const Lhs & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return operator<(rhs, lhs);
  }

  template<Integral Rhs, typename LhsInteger, u8_t LhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {
    return operator<(rhs, lhs);
  }

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>=(const fixed<LhsInteger, LhsFraction> & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return !operator<(lhs, rhs);
  }

  template<Integral Lhs, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>=(const Lhs & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return !operator<(lhs, rhs);
  }

  template<Integral Rhs, typename LhsInteger, u8_t LhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator>=(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {
    return !operator<(lhs, rhs);
  }

  template<typename LhsInteger, u8_t LhsFraction, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator<=(const fixed<LhsInteger, LhsFraction> & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return !operator<(rhs, lhs);
  }

  template<Integral Lhs, typename RhsInteger, u8_t RhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator<=(const Lhs & lhs, const fixed<RhsInteger, RhsFraction> & rhs) noexcept {
    return !operator<(rhs, lhs);
  }

  template<Integral Rhs, typename LhsInteger, u8_t LhsFraction>
  J_A(AI,ND,NODISC,HIDDEN) inline constexpr bool operator<=(const fixed<LhsInteger, LhsFraction> & lhs, const Rhs & rhs) noexcept {
    return !operator<(rhs, lhs);
  }

  using s16_16 = fixed<i32_t, 16>;
  using u16_16 = fixed<u32_t, 16>;

  using s24_8 = fixed<i32_t, 8>;
  using u24_8 = fixed<u32_t, 8>;

  using s8_24 = fixed<i32_t, 24>;
  using u8_24 = fixed<u32_t, 24>;

  using s32_32 = fixed<i64_t, 32>;
  using u32_32 = fixed<u64_t, 32>;
}
