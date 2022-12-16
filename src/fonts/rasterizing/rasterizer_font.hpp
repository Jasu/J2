#pragma once

#include "math/fixed.hpp"

namespace j::strings {
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
}

namespace j::fonts::rasterizing {
  /// Base class for rasterizer fonts.
  struct rasterizer_font {
    constexpr rasterizer_font() noexcept = default;

    virtual ~rasterizer_font();

    virtual strings::const_string_view family_name() const = 0;

    virtual strings::const_string_view style_name() const = 0;

    explicit rasterizer_font(math::s16_16 pixel_size);

    math::s16_16 pixel_size = 0;

    /// Line height in pixels.
    ///
    /// Line height is the distance between baselines.
    math::s16_16 line_height = 0;

    /// Nominal ascender height of the font in pixels.
    math::s16_16 ascent = 0;
    /// Nominal descender height of the font in pixels.
    math::s16_16 descent = 0;
    /// Suggested gap between consequetive lines in pixels.
    math::s16_16 line_gap = 0;

    /// Preferred underline position of the font in pixels.
    math::s16_16 underline_position = 0;
    /// Preferred underline thickness of the font in pixels.
    math::s16_16 underline_thickness = 0;

    /// Width of the space character.
    math::s16_16 space_width = 0;
  };
}
