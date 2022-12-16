#include "fonts/shaping/shaper.hpp"
#include "fonts/shaping/shaped_string.hpp"
#include "fonts/font_set.hpp"
#include "fonts/rasterizing/font_image_source_handler.hpp"
#include "fonts/rasterizing/font_rasterizer.hpp"
#include "fonts/rasterizing/freetype/ft_sized_font.hpp"
#include "containers/hash_map.hpp"
#include "services/service_definition.hpp"
#include "strings/unicode/rope/rope_utf8_view.hpp"

#include <ft2build.h>

#include FT_FREETYPE_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

namespace j::fonts::shaping {
  shaper::shaper(mem::shared_ptr<rasterizing::font_rasterizer> && rasterizer)
    : m_buffer(hb_buffer_create()),
      m_rasterizer(static_cast<mem::shared_ptr<rasterizing::font_rasterizer> &&>(rasterizer))
  {
    J_REQUIRE(hb_buffer_allocation_successful(m_buffer), "Allocating a HarfBuzz buffer failed.");
  }

  shaper::~shaper() {
    J_ASSERT_NOT_NULL(m_buffer);
    hb_buffer_destroy(m_buffer);
  }


  mem::shared_ptr<shaped_string> shaper::shape(strings::const_rope_utf8_view str,
                                               const font_set & font_selection, u32_t dpi) const
  {
    J_ASSERT(!font_selection.empty(), "Tried to shape with an empty font selection.");
    J_ASSERT_NOT_NULL(str);

    hb_buffer_reset(m_buffer);

    str.for_each_chunk([&](strings::const_utf8_string_view v) noexcept {
      J_ASSERT_NOT_NULL(v);
      hb_buffer_add_utf8(m_buffer, v.begin(), v.size(), 0, -1);
    });

    hb_buffer_guess_segment_properties(m_buffer);

    auto & font = font_selection[0];

    const FT_Face ft_font = reinterpret_cast<FT_Face>(m_rasterizer->get_font(font, dpi));
    J_ASSERT_NOT_NULL(ft_font);

    auto f = mem::reinterpret_pointer_cast<hb_font_t>(font.shaper_font());
    if (J_UNLIKELY(!f)) {
      f = mem::wrap_shared<hb_font_t, &hb_font_destroy>(hb_ft_font_create_referenced(ft_font));
      font.set_shaper_font(f);
    }
    hb_shape(f.get(), m_buffer, nullptr, 0);

    u32_t num_glyphs = 0U;
    const u32_t space_glyph = m_rasterizer->get_codepoint_glyph_index(font, ' ');

    auto infos = hb_buffer_get_glyph_infos(m_buffer, &num_glyphs);
    auto positions = hb_buffer_get_glyph_positions(m_buffer, &num_glyphs);
    J_ASSERT_NOT_NULL(infos, positions);


    hash_set<u32_t> unique_glyphs;
    u32_t num_non_empty_glyphs = 0U;
    for (u32_t i = 0U; i < num_glyphs; ++i) {
      if (infos[i].codepoint != space_glyph) {
        ++num_non_empty_glyphs;
        unique_glyphs.insert(infos[i].codepoint);
      }
    }

    constexpr float multiplier = 1.0f / (float)(1 << 6);

    const u16_t font_index = mem::static_pointer_cast<rasterizing::freetype::ft_sized_font>(font.rasterizer_font())->index;
    auto result = shaped_string::allocate(num_non_empty_glyphs, font_index, unique_glyphs.size());
    {
      J_ASSERT(result->unique_glyphs().size() == (u32_t)unique_glyphs.size(), "Out of range");
      auto uniques = result->unique_glyphs();
      u16_t i = 0U;
      for (auto g : unique_glyphs) {
        uniques[i++] = g;
      }
      J_ASSERT(i == result->unique_glyphs().size(), "Out of range");
    }
    float offset_x = 0.0f, offset_y = 0.0f, width = 0.0f, height = 0.0f;
    for (u32_t i = 0U, write_index = 0U; i < num_glyphs; ++i) {
      if (infos[i].codepoint != space_glyph) {
        const float cur_offset_x = offset_x + multiplier * positions[i].x_offset,
                    cur_offset_y = offset_y + multiplier * positions[i].y_offset;
        (*result)[write_index++] = { cur_offset_x, cur_offset_y, infos[i].codepoint };

        hb_glyph_extents_t extent;
        hb_font_get_glyph_extents(f.get(), infos[i].codepoint, &extent);
        width = max(width, cur_offset_x + extent.width * multiplier);
        height = max(height, cur_offset_y - extent.height * multiplier);
      }
      offset_x += multiplier * positions[i].x_advance;
      offset_y += multiplier * positions[i].y_advance;
    }
    result->set_size(width, height);
    return result;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<shaper> def(
      "fonts.shaping.shaper",
      "Text shaper",
      s::create = s::constructor<mem::shared_ptr<rasterizing::font_rasterizer> &&>()
    );
  }
}
