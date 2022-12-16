#pragma once

#include "rendering/images/image_buffer_info.hpp"

namespace j::rendering::images::detail {
  template<typename Ptr>
  class image_buffer_view {
  public:
    J_ALWAYS_INLINE constexpr image_buffer_view() noexcept = default;

    J_ALWAYS_INLINE constexpr image_buffer_view(image_buffer_info info, Ptr * J_NOT_NULL data) noexcept
      : info(info), data(data)
    { }

    constexpr image_buffer_view(
      u16_t width_pixels,
      u16_t height_pixels,
      i16_t stride_bytes,
      image_format format,
      Ptr * data) noexcept
      : image_buffer_view(
          image_buffer_info(width_pixels, height_pixels, stride_bytes, format),
          data)
    { }

    constexpr image_buffer_view(
      u16_t width_pixels,
      u16_t height_pixels,
      image_format format,
      Ptr * data) noexcept
      : image_buffer_view(
          image_buffer_info(width_pixels, height_pixels, format),
          data)
    { }

    constexpr image_buffer_view(u16_t size, image_format format, Ptr * data) noexcept
      : image_buffer_view(image_buffer_info(size, format), data)
    { }

    J_INLINE_GETTER constexpr bool empty() const noexcept {
      return !data;
    }

    J_INLINE_GETTER explicit constexpr operator bool() const noexcept {
      return data;
    }

    J_INLINE_GETTER constexpr bool operator!() const noexcept {
      return !data;
    }

    J_INLINE_GETTER operator image_buffer_view<const u8_t>() noexcept {
      return image_buffer_view<const u8_t>(info, data);
    }

    image_buffer_info info;
    Ptr * data = nullptr;
  };
}

extern template class j::rendering::images::detail::image_buffer_view<u8_t>;
extern template class j::rendering::images::detail::image_buffer_view<const u8_t>;

namespace j::rendering::images {
  using image_buffer_view = detail::image_buffer_view<u8_t>;
  using const_image_buffer_view = detail::image_buffer_view<const u8_t>;
}
