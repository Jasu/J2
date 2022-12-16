#include "ui/elements/text_view.hpp"
#include "ui/text/paragraph_layout_engine.hpp"
#include "ui/rendering/context.hpp"
#include "mem/shared_ptr.hpp"
#include "rendering/renderers/text/text_renderer.hpp"
#include "rendering/renderers/text/text_renderer_string.hpp"

namespace j::ui::elements {
  void text_view::render(context & ctx, const g::rect_u16 & region) const {
    m_frame.render(ctx, region, m_box.border, m_style);
    ctx.paragraph_layout_engine->initialize_line_table(m_lines, 0, ::j::max(1, m_str->num_hard_breaks()), m_fonts, *m_str);
    ctx.paragraph_layout_engine->format_line_table(m_lines, { (float)region.width() });
    bool is_first = true;
    float y_offset = 0;
    for (auto & l : m_lines) {
      for (auto & vl : l.visual_lines) {
        for (auto & w : vl.words) {
          mem::shared_ptr<fonts::shaping::shaped_string> string;
          for (auto & pw : l.physical_line) {
            if (pw.get() == w.string) {
              string = pw;
            }
          }

          g::vec2f pos(w.x + m_box.border.left, w.y + m_box.border.top + (is_first ? 0.5f * vl.gap : vl.gap) + y_offset);
          auto str = mem::make_shared<j::rendering::renderers::text::text_renderer_string>(
            ctx.text_renderer,
            pos,
            string,
            colors::rgba8(244,255,10));
          ctx.text_renderer->draw_string(*ctx.builder, str);
        }
        y_offset += vl.height;
        is_first = false;
      }
    }
  }
}
