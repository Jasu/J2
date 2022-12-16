#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/math.hpp"

namespace j::math {
  enum by_size_tag { by_size };
  enum by_endpoints_tag { by_endpoints };

  template<typename Num>
  struct interval final {
    using size_type = j::make_unsigned_t<Num>;

    static const inline Num min_v = j::min_value_v<Num>;
    static const inline Num max_v = j::max_value_v<Num>;
    static const inline size_type max_size_v = j::max_value_v<size_type>;

    J_ALWAYS_INLINE interval() noexcept = default;

    J_ALWAYS_INLINE interval(by_endpoints_tag, Num left, Num right) noexcept
      : m_left(left),
        m_size(sub_overflow(right, left))
    { }

    J_ALWAYS_INLINE interval(by_size_tag, Num left, size_type size) noexcept
      : m_left(size ? left : 0),
        m_size(size)
    { }

    J_INLINE_GETTER bool contains(Num num) const noexcept {
      return (size_type)(num - m_left) < m_size;
    }

    J_INLINE_GETTER bool contains(const interval & other) const noexcept {
      const size_type other_l = (size_type)other.m_left - (size_type)m_left;
      return (size_type)(other_l + other.m_size) <= m_size;
    }

    J_INLINE_GETTER bool intersects(const interval & other) const noexcept {
      const size_type other_l = (size_type)other.m_left - (size_type)m_left;
      const size_type other_r = other_l + other.m_size;
      return other_l < m_size || other_r < m_size;
    }

    J_INLINE_GETTER interval intersection(const interval & other) const {
      return interval(by_endpoints, j::max(m_left, other.m_left), j::min(right(), other.right()));
    }

    J_INLINE_GETTER Num min() const noexcept                         { return is_overflown() ? min_v : m_left; }
    J_INLINE_GETTER Num max_exclusive() const noexcept               { return add_sat(m_left, m_size); }

    J_A(AI,ND,NODISC) inline Num left() const noexcept                        { return m_left; }
    J_A(AI,ND,NODISC) inline Num right() const noexcept                       { return add_overflow(m_left, m_size); }
    J_A(AI,ND,NODISC) inline Num size() const noexcept                        { return m_size; }

    J_ALWAYS_INLINE void set_left(Num left) noexcept                 { m_left = left; }
    J_ALWAYS_INLINE void set_right(Num right) noexcept               { m_size = sub_overflow(right, m_left); }
    J_ALWAYS_INLINE void set_size(size_type size) noexcept            { m_size = size; }

    J_ALWAYS_INLINE void add_left(Num left) noexcept                 { m_left = add_overflow(m_left, left); }
    J_ALWAYS_INLINE void add_right(Num right) const noexcept         { m_size += right; }
    J_ALWAYS_INLINE void add_size(size_type size) const noexcept      { m_size += size; }

    J_ALWAYS_INLINE void subtract_left(Num left) noexcept            { m_left = sub_overflow(m_left, left); }
    J_ALWAYS_INLINE void subtract_right(Num right) const noexcept    { m_size -= right; }
    J_ALWAYS_INLINE void subtract_size(size_type size) const noexcept { m_size -= size; }

    inline void clamp_left(Num left) noexcept {
      const auto r = right();
      m_left = j::clamp(m_left, left, r);
      set_right(r);
    }
    inline void clamp_right(Num right) noexcept {
      m_size = j::clamp(m_left, right, m_size + m_left) - m_left;
    }
    inline void clamp_size(size_type size) noexcept {
      m_size = j::max(m_size, size);
    }

    J_INLINE_GETTER interval operator+(Num rhs) const noexcept {
      return interval(by_size, add_overflow(m_left, rhs), m_size);
    }

    J_INLINE_GETTER interval operator-(Num rhs) const noexcept {
      return interval(by_size, sub_overflow(m_left, rhs), m_size);
    }

    J_INLINE_GETTER interval operator*(Num rhs) const noexcept {
      return interval(by_endpoints, mul_overflow(m_left, rhs), mul_sat(m_size, rhs));
    }

    [[nodiscard]] inline interval operator/(Num rhs) const noexcept {
      J_ASSERT(rhs != 0);
      return interval(by_size, m_left / rhs, m_size / (size_type)abs(rhs));
    }

    J_INLINE_GETTER interval operator+(interval rhs) const noexcept {
      return interval(by_size, add_overflow(m_left, rhs.m_left), m_size + rhs.m_size);
    }

    J_INLINE_GETTER interval operator-(interval rhs) const noexcept {
      return interval(by_size, sub_overflow(m_left, rhs.m_left), m_size + rhs.m_size);
    }

    [[nodiscard]] inline interval operator*(interval rhs) const noexcept {
      return interval(by_endpoints, mul_overflow(min(), rhs.min()), mul_sat(max_exclusive(), rhs.max_exclusive()));
    }

    J_A(ND,NODISC) inline bool is_overflown() const noexcept      { return add_overflows(m_left, m_size); }
    J_A(AI,ND,NODISC) inline bool operator!() const noexcept         { return !m_size; }
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return m_size; }

    J_A(AI,NODISC) inline bool operator==(const interval & rhs) const noexcept {
      return m_size == rhs.m_size && (m_left == rhs.m_left || !m_size);
    }
  private:
    Num m_left = 0;
    size_type m_size = 0;
  };

  using interval_i8  = interval<i8_t>;
  using interval_i32 = interval<i32_t>;

  using interval_u8  = interval<u8_t>;
  using interval_u32 = interval<u32_t>;
}
