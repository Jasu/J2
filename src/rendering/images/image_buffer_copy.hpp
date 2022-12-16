#pragma once

#include "rendering/images/image_buffer_view.hpp"

namespace j::rendering::images {
  namespace detail {
    void copy_direct(
      u8_t * to,
      const u8_t * from,
      u16_t width_bytes,
      u16_t height) noexcept;

    void copy_strided(
      u8_t * to,
      const u8_t * from,
      u16_t width_bytes,
      u16_t height,
      i16_t to_stride,
      i16_t from_stride) noexcept;

    void copy_convert(
      u8_t * to,
      const u8_t * from,
      u16_t width,
      u16_t height,
      i16_t to_stride,
      i16_t from_stride,
      image_format to_format,
      image_format from_format);
  }

  template<typename From>
  void image_buffer_copy(const image_buffer_view & to, From && from) {
    u8_t * const to_data = to.data;
    const u8_t * const from_data = from.data;
    J_ASSERT_NOT_NULL(to_data, from_data);
    J_ASSERT(to_data != from_data, "Cannot assign image buffer to self.");

    const u16_t width = to.info.width(), height = to.info.height();
    J_ASSERT_NOT_NULL(width, height);
    J_ASSERT(width == from.info.width() && height == from.info.height(), "Target and source buffer sizes do not match.");

    J_ASSERT(to.info.dimensionality() == from.info.dimensionality(),
             "Dimensions mismatch");

    if (to.info.dimensionality() == 1U) {
      if (to.info.format() == from.info.format()) {
        const u16_t width_bytes = to.info.width() * bytes_per_pixel(to.info.format());
        detail::copy_direct(to_data, from_data, width_bytes, 1);
      } else {
        detail::copy_convert(to_data, from_data, width, height, 1, 1, to.info.format(), from.info.format());
      }
      return;
    }

    const i16_t to_stride = to.info.stride(), from_stride = from.info.stride();

    J_ASSERT_NOT_NULL(to_stride, from_stride);

    if (to.info.format() == from.info.format()) {
      const u16_t width_bytes = to.info.width() * bytes_per_pixel(to.info.format());
      if (to_stride == from_stride && to_stride == width_bytes) {
        detail::copy_direct(to_data, from_data, width_bytes, height);
      } else {
        detail::copy_strided(to_data, from_data, width_bytes, height, to_stride, from_stride);
      }
    } else {
      detail::copy_convert(to_data, from_data, width, height, to_stride, from_stride, to.info.format(), from.info.format());
    }
  }
}
