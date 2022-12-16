#pragma once

#include "math/fixed.hpp"

namespace j::fonts::rasterizing {
  class glyph_info final {
  public:
    glyph_info() noexcept = default;

    glyph_info(
      u16_t width_px,
      u16_t height_px,
      math::s16_16 x_bearing,
      math::s16_16 y_bearing,
      bool is_color) noexcept
    : m_width_px(width_px),
      m_height_px(height_px),
      m_is_color(is_color),
      m_x_bearing(x_bearing),
      m_y_bearing(y_bearing)
    {
    }

    u16_t width() const noexcept {
      return m_width_px;
    }

    u16_t height() const noexcept {
      return m_height_px;
    }

    math::s16_16 x_bearing() const noexcept {
      return m_x_bearing;
    }

    math::s16_16 y_bearing() const noexcept {
      return m_y_bearing;
    }

    bool is_color() const noexcept {
      return m_is_color;
    }
  private:
    u16_t m_width_px;
    u16_t m_height_px:15;
    bool m_is_color:1;
    math::s16_16 m_x_bearing;
    math::s16_16 m_y_bearing;
  };
}
