#pragma once

#include "ui/element.hpp"

#include "ui/rendering/element_frame.hpp"
#include "ui/text/line_table.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"
#include "fonts/font_set.hpp"

namespace j::ui::elements {
  /// A static element that only draws its frame.
  class text_view final : public static_element {
  public:
    using static_element::static_element;
    J_ALWAYS_INLINE void set_text(const strings::rope_utf8 * view) noexcept {
      m_str = view;
    }
    J_ALWAYS_INLINE void set_fonts(const fonts::font_set & fonts) noexcept {
      m_fonts = fonts;
    }
    void render(context & ctx, const g::rect_u16 & region) const override;
  private:
    element_frame m_frame;
    const strings::rope_utf8 * m_str = nullptr;
    mutable line_table m_lines;
    fonts::font_set m_fonts;
  };
}
