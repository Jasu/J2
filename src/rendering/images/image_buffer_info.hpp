#pragma once

#include "rendering/images/image_format.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::images {
  class image_buffer_info final {
    u16_t m_width = 0U;
    u16_t m_height = 0U;
    u16_t m_bearing_x = 0U;
    u16_t m_bearing_y = 0U;
    i16_t m_stride_bytes = 0U;
    image_format m_format = image_format::gray8;
    u8_t m_dimensionality = 2U;
  public:
    constexpr image_buffer_info() noexcept = default;

    constexpr image_buffer_info(u16_t width, u16_t height, i16_t stride_bytes, image_format format, u16_t bearing_x = 0, u16_t bearing_y = 0) noexcept
      : m_width(width),
        m_height(height),
        m_bearing_x(bearing_x),
        m_bearing_y(bearing_y),
        m_stride_bytes(stride_bytes),
        m_format(format)
    {
      J_ASSUME(width != 0);
      J_ASSUME(height != 0);
      J_ASSUME(stride_bytes != 0);
      J_ASSERT(stride_bytes >= width || -stride_bytes >= width, "Stride must be greater than or equal to width.");
    }

    constexpr image_buffer_info(u16_t size, image_format format) noexcept
      : m_width(size),
        m_height(1U),
        m_format(format),
        m_dimensionality(1U)
    {
      J_ASSUME(size != 0);
    }

    constexpr image_buffer_info(u16_t width, u16_t height, image_format format, u16_t bearing_x = 0, u16_t bearing_y = 0) noexcept
      : image_buffer_info(width, height, width * bytes_per_pixel(format), format, bearing_x, bearing_y)
    {
    }

    constexpr bool empty() const noexcept {
      return m_width == 0;
    }

    explicit constexpr operator bool() const noexcept {
      return m_width != 0;
    }

    constexpr bool operator!() const noexcept {
      return m_width == 0;
    }

    constexpr u16_t width() const noexcept {
      return m_width;
    }

    constexpr u16_t height() const noexcept {
      return m_height;
    }

    constexpr u16_t bearing_x() const noexcept {
      return m_bearing_x;
    }

    constexpr u16_t bearing_y() const noexcept {
      return m_bearing_y;
    }

    constexpr i16_t stride() const noexcept {
      return m_stride_bytes;
    }

    constexpr image_format format() const noexcept {
      return m_format;
    }

    constexpr u8_t dimensionality() const noexcept {
      return m_dimensionality;
    }

    constexpr u32_t size_bytes() const noexcept {
      return m_height * m_width * bytes_per_pixel(m_format);
    }
  };
}
