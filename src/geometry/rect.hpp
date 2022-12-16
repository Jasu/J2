#pragma once

#include "geometry/axis.hpp"
#include "geometry/perimeter.hpp"
#include "geometry/vec2.hpp"
#include "hzd/integer_traits.hpp"

namespace j::geometry {
  template<typename Num>
  class rect {
    using UNum = ::j::make_unsigned_t<Num>;
    vec2<Num> m_nw;
    vec2<UNum> m_size;

  public:
    J_ALWAYS_INLINE_NO_DEBUG rect() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG constexpr rect(Num x, Num y, UNum w, UNum h)      noexcept
      : m_nw{x, y},   m_size{w, h} { }

    J_ALWAYS_INLINE constexpr rect(vec2<Num> nw, vec2<UNum> size)     noexcept
      : m_nw{nw},     m_size{size} { }

    J_ALWAYS_INLINE constexpr rect(Num x, Num y, vec2<UNum> size)     noexcept
      : m_nw{x, y},   m_size{size} { }

    J_ALWAYS_INLINE constexpr rect(vec2<Num> nw, UNum w, UNum h)      noexcept
      : m_nw{nw},     m_size{w, h} { }

    template<typename N2>
    J_INLINE_GETTER operator rect<N2>() const noexcept {
      return rect<N2>{m_nw, m_size};
    }



    J_INLINE_GETTER constexpr vec2<Num> ne()    const noexcept { return vec2<Num>{ m_nw.x + m_size.x, m_nw.y }; }
    J_INLINE_GETTER constexpr vec2<Num> se()    const noexcept { return m_nw + m_size; }
    J_INLINE_GETTER constexpr vec2<Num> sw()    const noexcept { return vec2<Num>{ m_nw.x, m_nw.y + m_size.y }; }
    J_INLINE_GETTER constexpr vec2<Num> nw()    const noexcept { return m_nw; }

    J_INLINE_GETTER constexpr vec2<UNum> size() const noexcept { return m_size; }

    J_INLINE_GETTER constexpr Num start(axis a) const noexcept {
      return m_nw.at(a);
    }
    J_INLINE_GETTER constexpr Num end(axis a) const noexcept {
      return m_nw.at(a) + m_size.at(a);
    }
    J_INLINE_GETTER constexpr UNum size(axis a) const noexcept {
      return m_size.at(a);
    }

    J_INLINE_GETTER constexpr UNum width()      const noexcept { return m_size.x; }
    J_INLINE_GETTER constexpr UNum height()     const noexcept { return m_size.y; }

    J_INLINE_GETTER constexpr Num left()        const noexcept { return m_nw.x; }
    J_INLINE_GETTER constexpr Num right()       const noexcept { return m_nw.x + m_size.x; }
    J_INLINE_GETTER constexpr Num top()         const noexcept { return m_nw.y; }
    J_INLINE_GETTER constexpr Num bottom()      const noexcept { return m_nw.y + m_size.y; }

    J_ALWAYS_INLINE constexpr void set_left(Num v)    noexcept { m_nw.x = v; }
    J_ALWAYS_INLINE constexpr void set_top(Num v)     noexcept { m_nw.y = v; }
    J_ALWAYS_INLINE constexpr void set_width(UNum v)  noexcept { m_size.x = v; }
    J_ALWAYS_INLINE constexpr void set_height(UNum v) noexcept { m_size.y = v; }

    template<typename PNum>
    constexpr rect & operator+=(perimeter<PNum> perimeter) const noexcept {
      m_nw.x -= perimeter.left, m_nw.y -= perimeter.top,
      m_size.x += perimeter.left + perimeter.right,
      m_size.y += perimeter.top + perimeter.bottom;
      return *this;
    }

    template<typename PNum>
    constexpr rect & operator-=(perimeter<PNum> perimeter) const noexcept {
      m_nw.x += perimeter.left, m_nw.y += perimeter.top,
      m_size.x -= perimeter.left + perimeter.right,
      m_size.y -= perimeter.top + perimeter.bottom;
      return *this;
    }

    J_INLINE_GETTER constexpr bool empty()      const noexcept { return !m_size.x || !m_size.y; }

    J_INLINE_GETTER explicit constexpr operator bool() const noexcept {
      return !empty();
    }

    J_INLINE_GETTER constexpr bool operator!() const noexcept {
      return empty();
    }

    J_ALWAYS_INLINE constexpr bool operator==(const rect & r) const noexcept = default;
  };

  template<typename RNum, typename PNum>
  constexpr inline rect<RNum> operator+(const rect<RNum> & lhs, const perimeter<PNum> & rhs) noexcept {
    return rect<RNum>{
      lhs.left() - rhs.left,
      lhs.top() - rhs.top,
      lhs.width() + rhs.left + rhs.right,
      lhs.height() + rhs.top + rhs.bottom
    };
  }

  template<typename RNum, typename PNum>
  constexpr inline rect<RNum> operator-(const rect<RNum> & lhs, const perimeter<PNum> & rhs) noexcept {
    return rect<RNum>{
      lhs.left() + rhs.left,
      lhs.top() + rhs.top,
      lhs.width() - rhs.left - rhs.right,
      lhs.height() - rhs.top - rhs.bottom
    };
  }

  template<typename RNum, typename PNum>
  J_INLINE_GETTER constexpr rect<RNum> operator+(const perimeter<PNum> & lhs, rect<RNum> rhs) noexcept {
    return rhs + lhs;
  }

  template<typename RNum, typename PNum>
  J_INLINE_GETTER constexpr rect<RNum> operator-(perimeter<PNum> lhs, rect<RNum> rhs) noexcept {
    return rhs - lhs;
  }

  using rect_float = rect<float>;

  using rect_i16 = rect<i16_t>;
  using rect_i32 = rect<i32_t>;

  using rect_u16 = rect<u16_t>;
  using rect_u32 = rect<u32_t>;
}
