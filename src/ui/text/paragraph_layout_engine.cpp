#include "ui/text/paragraph_layout_engine.hpp"

#include "ui/text/line_table.hpp"
#include "fonts/rasterizing/font_rasterizer.hpp"
#include "fonts/rasterizing/rasterizer_font.hpp"
#include "fonts/shaping/shaper.hpp"
#include "fonts/shaping/shaped_string.hpp"
#include "services/service_definition.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/unicode/rope/rope_utf8_lbr_iterator.hpp"

namespace j::ui::inline text {
  namespace s = strings;
  namespace c = containers;
  namespace f = fonts;
  namespace fs = fonts::shaping;
  namespace fr = fonts::rasterizing;

  namespace {
    struct J_TYPE_HIDDEN line_state final {
      float max_gap = 0.0f;
      float max_ascent = 0.0f;
      float min_descent = 0.0f;
      float width_left = 0.0f;

      void reset(const layout_options & opts) {
        max_gap = 0.0f;
        max_ascent = 0.0f;
        min_descent = 0.0f;
        width_left = opts.max_width;
      }

      line_state(const layout_options & opts) noexcept {
        reset(opts);
      }

      void handle_font(const fr::rasterizer_font & font) noexcept {
        max_gap = ::j::max(max_gap, font.line_gap.as_float());
        min_descent = ::j::min(min_descent, font.descent.as_float());
        max_ascent = ::j::max(max_ascent, font.ascent.as_float());
      }

      void append(
        c::vector<visual_line> & result,
        fr::font_rasterizer & rast,
        const mem::shared_ptr<fonts::shaping::shaped_string> * & previous,
        const mem::shared_ptr<fonts::shaping::shaped_string> * cur
      ) const {
        positioned_strings_t pos(cur - previous);
        float x = 0.0f;
        for (positioned_string * target = pos.begin(); previous != cur; ++previous, ++target) {
          auto font_info = rast.get_rasterizer_font((*previous)->font_index());
          *target = positioned_string{
            previous->get(),
            x,
            // (max_ascent - ascent) - ascent: Move down to match the ascent of a string
            // drawn with a font that has a smaller (== larger, they are negative) ascent.
            // Then, normalize to positive number by subtracting the ascent once more.
            max_ascent};
          x += (*previous)->width() + font_info->space_width.as_float();
        }

        result.push_back({
            max_gap,
            max_ascent - min_descent,
            static_cast<positioned_strings_t &&>(pos)});
      }
    };
  }

  paragraph_layout_engine::paragraph_layout_engine(
    mem::shared_ptr<fr::font_rasterizer> && rast,
    mem::shared_ptr<fonts::shaping::shaper> && shaper) noexcept
    : m_rasterizer(static_cast<mem::shared_ptr<fr::font_rasterizer> &&>(rast)),
      m_shaper(static_cast<mem::shared_ptr<fs::shaper> &&>(shaper))
  {
    J_ASSERT_NOT_NULL(m_rasterizer, m_shaper);
  }

  void paragraph_layout_engine::initialize_line_table(
    line_table & table, u32_t first_line, u32_t num_lines,
    const f::font_set & fonts, const strings::rope_utf8 & rope
  ) const {
    table.clear();
    table.set_first_line_number(first_line);
    s::rope_utf8_lbr_iterator cur(rope.find_line(first_line)), prev = cur, end(rope.code_points().end());
    for (u32_t i = 0U; i < num_lines; ++i) {
      line_t line;
      do {
        prev = cur++;
        auto prev_it = (s::rope_utf8_code_point_iterator)(cur), end_it = prev_it--;
        while ((s::const_rope_utf8_byte_iterator)prev != (s::const_rope_utf8_byte_iterator)prev_it
                && (*prev_it == '\n' || *prev_it == '\r')) {
          end_it = prev_it--;
        }
        line.push_back(m_shaper->shape(rope.view_at(prev, end_it), fonts, 72U));
      } while (cur != end && cur->break_type != s::line_break_type::mandatory);
      table.append_line({ static_cast<line_t &&>(line), {} });
    }
  }

  void paragraph_layout_engine::format_line_table(line_table & table,
                                                  const layout_options & opts) const
  {
    for (auto & l : table) {
      l.visual_lines = format_line(opts, l.physical_line);
    }
  }


  c::vector<visual_line> paragraph_layout_engine::format_line(const layout_options & opts,
                                                              const line_t & line) const {
    c::vector<visual_line> result;
    const auto end = line.end();
    auto previous = line.begin(), cur = previous;

    bool is_first = true;
    line_state state(opts);
    for (; cur != end; ++cur) {
      auto font_info = m_rasterizer->get_rasterizer_font((*cur)->font_index());
      float width = (*cur)->width();
      if (!is_first) {

        // Fits with the space, continue.
        const float width_taken = width + font_info->space_width.as_float();
        if (width_taken <= state.width_left) {
          state.width_left -= width_taken;
          continue;
        }

        state.append(result, *m_rasterizer, previous, cur);
        state.reset(opts);
      }

      state.width_left -= width;
      state.handle_font(*font_info);
      is_first = false;
    }
    if (previous != cur) {
      state.append(result, *m_rasterizer, previous, cur);
    }
    return result;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<paragraph_layout_engine> def(
      "ui.text.paragraph_layout_engine",
      "Paragraph layout engine",
      s::create = s::constructor<
        mem::shared_ptr<fr::font_rasterizer> &&,
        mem::shared_ptr<fs::shaper> &&
      >()
    );
  }
}
