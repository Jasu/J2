#pragma once

#include "mem/shared_ptr_fwd.hpp"

extern "C" {
  struct hb_buffer_t;
}

namespace j::inline containers {
  template<typename T> class vector;
}

namespace j::strings::inline unicode::inline rope {
  template<bool> class basic_rope_utf8_view;
  using const_rope_utf8_view = basic_rope_utf8_view<true>;
}

namespace j::fonts {
  class font;
  using font_set = vector<font>;
}
namespace j::fonts::rasterizing {
  class glyph_key;
  class font_rasterizer;
}

namespace j::fonts::shaping {
  class shaped_string;

  /// Shape text into positioned glyphs.
  class shaper final {
  public:
    shaper(mem::shared_ptr<rasterizing::font_rasterizer> && rasterizer);

    ~shaper();

    mem::shared_ptr<shaped_string> shape(
      strings::const_rope_utf8_view str,
      const font_set & font_selection, u32_t dpi) const;

    shaper(const shaper &) = delete;
    shaper & operator=(const shaper &) = delete;
  private:
    hb_buffer_t * m_buffer;

    mem::shared_ptr<rasterizing::font_rasterizer> m_rasterizer;
  };
}
