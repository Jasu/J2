#pragma once

#include "hzd/types.hpp"
#include "rendering/images/image_buffer_view.hpp"

namespace j::fonts {
  class font;
  class font_size;
}
namespace j::strings {
  class string;
}
namespace j::mem {
  template<typename> class shared_ptr;
}

namespace j::fonts::rasterizing {
  class glyph_info;
  class glyph_key;
  struct rasterizer_font;

  class font_rasterizer {
  public:
    font_rasterizer() = default;

    virtual ~font_rasterizer();

    /// Get the shaper font of [f] at the specified point or pixel size.
    ///
    /// Return the font loaded.
    virtual void * get_font(const font & f, u32_t dpi) = 0;

    virtual mem::shared_ptr<rasterizer_font> get_rasterizer_font(u16_t font_index) const = 0;

    /// Get glyph info a the size and DPI specified.
    virtual glyph_info get_glyph_info(const glyph_key & key) = 0;

    virtual strings::string get_glyph_name(const glyph_key & key) const = 0;

    virtual u32_t get_codepoint_glyph_index(const font & f, u32_t codepoint) = 0;

    /// Rasterize glyph to target.
    virtual void rasterize_glyph(const glyph_key & key, rendering::images::image_buffer_view target) = 0;

    font_rasterizer(const font_rasterizer &) = delete;
    font_rasterizer & operator=(const font_rasterizer &) = delete;
  };
}
