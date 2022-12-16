#pragma once

#include "ui/text/line.hpp"

namespace j::strings::inline unicode::inline rope {
  template<bool> class basic_rope_utf8_view;
  using const_rope_utf8_view = basic_rope_utf8_view<true>;
  class rope_utf8;
}

namespace j::fonts {
  class font;
  using font_set = vector<font>;
}

namespace j::fonts::shaping {
  class shaper;
}

namespace j::fonts::rasterizing {
  class font_rasterizer;
}

namespace j::ui::inline text {
  class line_table;

  struct layout_options {
    float max_width;
  };

  class paragraph_layout_engine {
  public:
    paragraph_layout_engine(mem::shared_ptr<fonts::rasterizing::font_rasterizer> && rast,
                            mem::shared_ptr<fonts::shaping::shaper> && shaper) noexcept;

    void initialize_line_table(line_table & table, u32_t first_line, u32_t num_lines,
                               const fonts::font_set & fonst, const strings::rope_utf8 & rope) const;

    void format_line_table(line_table & table, const layout_options & opts) const;

    vector<visual_line> format_line(const layout_options & opts, const line_t & line) const;

  private:
    mem::shared_ptr<fonts::rasterizing::font_rasterizer> m_rasterizer;
    mem::shared_ptr<fonts::shaping::shaper> m_shaper;
  };
}
