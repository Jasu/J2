#pragma once

#include "hzd/type_traits.hpp"

namespace j::properties::access {
  class typed_access_definition;
}

namespace j::colors {
  extern const properties::access::typed_access_definition & rgb_access_definition;

  template<typename Component>
  struct rgb {
    Component r, g, b;

    J_ALWAYS_INLINE_NO_DEBUG rgb() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr rgb(u32_t color) noexcept
      : r(color >> 16), g(color >> 8), b(color) { }

    J_ALWAYS_INLINE_NO_DEBUG constexpr rgb(Component r, Component g, Component b) noexcept
      : r(r), g(g), b(b) { }

    J_ALWAYS_INLINE constexpr bool operator==(const rgb<Component> &) const noexcept = default;
  };

  template<typename Component>
  struct rgba {
    Component r, g, b, a;

    J_ALWAYS_INLINE_NO_DEBUG rgba() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG constexpr rgba(
      Component r,
      Component g,
      Component b,
      Component a = is_integral_v<Component> ? Component(255) : Component(1.0)
    ) noexcept : r(r), g(g), b(b), a(a) { }

    explicit constexpr rgba(
      rgb<Component> rgb,
      Component a = is_integral_v<Component> ? Component(255) : Component(1.0)
    ) noexcept
      : r(rgb.r), g(rgb.g), b(rgb.b), a(a) { }

    J_ALWAYS_INLINE constexpr bool operator==(const rgba<Component> & rhs) const noexcept = default;

    J_ALWAYS_INLINE rgb<Component> rgb() const noexcept {
      return {r, g, b};
    }
  };

  using rgb8 = rgb<u8_t>;
  using rgbf = rgb<float>;

  using rgba8 = rgba<u8_t>;
  using rgbaf = rgba<float>;

  J_A(AI,ND,HIDDEN,NODISC) inline rgbf rgb_to_float(rgb8 value) noexcept {
    constexpr float m = 0.0039215686f;
    return rgbf{ m * value.r, m * value.g, m * value.b };
  }

  J_A(AI,ND,HIDDEN,NODISC) inline rgb8 rgb_to_u8(const rgbf & value) noexcept {
    return rgb8(255U * value.r, 255U * value.g, 255U * value.b);
  }

  J_A(AI,ND,HIDDEN,NODISC) inline rgbaf rgba_to_float(rgba8 value) noexcept {
    constexpr float m = 0.0039215686f;
    return rgbaf{ m * value.r, m * value.g, m * value.b, m * value.a };
  }

  J_A(AI,ND,HIDDEN,NODISC) inline rgba8 rgba_to_u8(const rgbaf & value) noexcept {
    return rgba8(255U * value.r, 255U * value.g, 255U * value.b, 255U * value.a);
  }
}
