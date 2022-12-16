#pragma once

#include "hzd/utility.hpp"

namespace j::rendering::images {
  static inline constexpr u8_t rgb_mask        = 1U << 7U;
  static inline constexpr u8_t alpha_mask      = 1U << 6U;
  static inline constexpr u8_t gray_mask       = 1U << 5U;
  static inline constexpr u8_t order_rgba_mask = 1U << 4U;

  enum class image_format : u8_t {
    gray8  = 1U | gray_mask,
    rgb24  = 3U | rgb_mask | order_rgba_mask,
    bgr24  = 3U | rgb_mask,
    rgba32 = 4U | rgb_mask | order_rgba_mask | alpha_mask,
    bgra32 = 4U | rgb_mask | alpha_mask,
  };

  J_INLINE_GETTER constexpr u8_t bytes_per_pixel(image_format format) noexcept {
    return u8_t(format) & 0x7;
  }

  J_INLINE_GETTER constexpr bool has_alpha(image_format format) noexcept {
    return static_cast<u8_t>(format) & alpha_mask;
  }

  J_INLINE_GETTER constexpr bool is_color(image_format format) noexcept {
    return static_cast<u8_t>(format) & rgb_mask;
  }

  J_INLINE_GETTER constexpr bool is_grayscale(image_format format) noexcept {
    return static_cast<u8_t>(format) & gray_mask;
  }

  J_INLINE_GETTER constexpr bool is_rgba_order(image_format format) noexcept {
    return static_cast<u8_t>(format) & order_rgba_mask;
  }

  J_INLINE_GETTER constexpr bool is_bgra_order(image_format format) noexcept {
    return !is_rgba_order(format);
  }

  J_INLINE_GETTER constexpr bool is_convertible(image_format to, image_format from) noexcept {
    return to == from || !is_grayscale(to) || is_grayscale(from);
  }
}
