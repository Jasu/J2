#include "fonts/rasterizing/freetype/ft_rasterizer.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#include "fonts/rasterizing/freetype/ft_font.hpp"
#include "fonts/rasterizing/freetype/ft_sized_font.hpp"
#include "fonts/rasterizing/freetype/assert.hpp"
#include "fonts/rasterizing/glyph_info.hpp"
#include "fonts/rasterizing/glyph_key.hpp"
#include "fonts/font.hpp"
#include "fonts/font_size.hpp"
#include "files/paths/path_map.hpp"
#include "containers/hash_map.hpp"
#include "files/paths/path.hpp"
#include "mem/weak_ptr.hpp"
#include "services/services.hpp"

J_DEFINE_EXTERN_HASH_MAP(u16_t, j::mem::weak_ptr<j::fonts::rasterizing::freetype::ft_sized_font>);
J_DEFINE_EXTERN_PATH_MAP(j::mem::weak_ptr<j::fonts::rasterizing::freetype::ft_font>);

namespace j::fonts::rasterizing::freetype {
  namespace {
    void rasterize_glyph_impl(FT_Face font, u32_t glyph_index, u8_t * target, u32_t stride) {
      // if (!font->glyph || font->glyph->glyph_index != glyph_index) {
        J_FT_CALL_CHECKED(FT_Load_Glyph, font, glyph_index, FT_LOAD_DEFAULT);
      // }

      if (J_LIKELY(font->glyph->format != FT_GLYPH_FORMAT_BITMAP)) {
        J_FT_CALL_CHECKED(FT_Render_Glyph, font->glyph, FT_RENDER_MODE_NORMAL);
      }
      J_ASSERT(font->glyph->format == FT_GLYPH_FORMAT_BITMAP,
              "Expected the rendered glyph to be a bitmap.");

      stride -= font->glyph->bitmap.width * 4;
      const auto bytes_per_pixel = (font->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA ? 4 : 1);
      const auto src_stride = font->glyph->bitmap.pitch - bytes_per_pixel * font->glyph->bitmap.width;
      auto src_ptr = font->glyph->bitmap.buffer;
      for (u32_t y = 0; y < font->glyph->bitmap.rows; ++y) {
        for (u32_t x = 0; x < font->glyph->bitmap.width; ++x) {
          switch (font->glyph->bitmap.pixel_mode) {
          case FT_PIXEL_MODE_BGRA:
            target[0] = src_ptr[2];
            target[1] = src_ptr[1];
            target[2] = src_ptr[0];
            target[3] = src_ptr[3];
            src_ptr += 4;
            break;
          case FT_PIXEL_MODE_GRAY:
            target[0] = 255;
            target[1] = 255;
            target[2] = 255;
            target[3] = *src_ptr++;
            break;
          default:
            J_THROW("Unsupported bitmap pixel mode.");
          }
          target += 4;
        }
        target += stride;
        src_ptr += src_stride;
      }
    }
  }

  ft_rasterizer::ft_rasterizer() noexcept {
    J_FT_CALL_CHECKED(FT_Init_FreeType, reinterpret_cast<FT_Library*>(&m_library));
  }

  ft_rasterizer::~ft_rasterizer() {
    J_FT_CALL_CHECKED(FT_Done_FreeType, reinterpret_cast<FT_Library>(m_library));
  }

  void * ft_rasterizer::get_font(const font & f, u32_t dpi) {
    math::s16_16 pixel_size{ f.size().size() };
    if (!f.size().is_pixel_size()) {
      pixel_size = (pixel_size * dpi + 36) / 72;
    }
    mem::shared_ptr<ft_sized_font> size_rec
      = mem::static_pointer_cast<ft_sized_font>(f.rasterizer_font());
    if (!size_rec) {
      J_ASSERT_NOT_NULL(f.selector_font());
      const auto & path = *reinterpret_cast<files::path*>(f.selector_font().get());
      auto font = m_fonts.maybe_at(path);
      mem::shared_ptr<ft_font> font_rec = font ? font->lock() : mem::shared_ptr<ft_font>{};
      const u32_t * size_idx = nullptr;
      if (!font_rec) {
        FT_Face face = nullptr;
        J_FT_CALL_CHECKED(FT_New_Face, reinterpret_cast<FT_Library>(m_library), path.as_c_string(), 0, &face);
        J_ASSERT_NOT_NULL(face);
        font_rec = mem::make_shared<ft_font>(face, shared_from_this(), path);
        m_fonts.emplace(path, font_rec);
      } else {
        size_idx = font_rec->get_sized_font(pixel_size);
        if (J_LIKELY(size_idx)) {
          size_rec = m_sized_fonts.at(*size_idx).lock();
        }
      }
      if (!size_rec) {
        size_rec = mem::make_shared<ft_sized_font>(font_rec, pixel_size, size_idx ? *size_idx : m_font_index);
        if (size_idx) {
          m_sized_fonts.emplace(*size_idx, size_rec).first->second = size_rec;
        } else {
          [[maybe_unused]] auto p = m_sized_fonts.emplace(m_font_index, size_rec);
          J_ASSERT(p.second, "Added a font twice.");
          font_rec->add_sized_font(pixel_size, m_font_index++);
        }
      }
      f.set_rasterizer_font(size_rec);
    }
    return size_rec->face();
  }

  u32_t ft_rasterizer::get_codepoint_glyph_index(const font & f, u32_t codepoint) {
    FT_Face face = reinterpret_cast<FT_Face>(get_font(f, 72));
    J_ASSERT_NOT_NULL(face);
    return FT_Get_Char_Index(face, codepoint);
  }

  glyph_info ft_rasterizer::get_glyph_info(const glyph_key & key) {
    auto rec = m_sized_fonts.at(key.font_index()).lock();
    J_ASSERT_NOT_NULL(rec);
    auto font = rec->face();
    J_ASSERT_NOT_NULL(font);
    J_FT_CALL_CHECKED(FT_Activate_Size, rec->size);
    // if (!font->glyph || font->glyph->glyph_index != key.glyph_index()) {
      J_FT_CALL_CHECKED(FT_Load_Glyph, font, key.glyph_index(), FT_LOAD_DEFAULT);
    // }
    if (J_UNLIKELY(font->glyph->format == FT_GLYPH_FORMAT_BITMAP)) {
      return {
        (u16_t)font->glyph->bitmap.width,
        (u16_t)font->glyph->bitmap.rows,
        math::s16_16(font->glyph->metrics.horiBearingX << 10, math::raw_tag),
        math::s16_16(font->glyph->metrics.horiBearingY << 10, math::raw_tag),
        font->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA,
      };
    } else {
      return {
        (u16_t)((font->glyph->metrics.width + 63) / 64),
        (u16_t)((font->glyph->metrics.height + 63) / 64),
        math::s16_16(font->glyph->metrics.horiBearingX << 10, math::raw_tag),
        math::s16_16(font->glyph->metrics.horiBearingY << 10, math::raw_tag),
        false
      };
    }
  }

  mem::shared_ptr<rasterizer_font> ft_rasterizer::get_rasterizer_font(u16_t font_index) const {
    auto fnt = m_sized_fonts.maybe_at(font_index);
    return fnt ? fnt->lock() : mem::shared_ptr<ft_sized_font>{};
  }

  strings::string ft_rasterizer::get_glyph_name(const glyph_key & key) const {
    auto rec = m_sized_fonts.at(key.font_index()).lock();
    J_ASSERT_NOT_NULL(rec);
    auto font = rec->face();
    J_ASSERT_NOT_NULL(font);
    char buf[256];
    J_FT_CALL_CHECKED(FT_Get_Glyph_Name, font, key.glyph_index(), buf, 256);
    return strings::string((const char*)buf);
  }

  void ft_rasterizer::rasterize_glyph(
    const glyph_key & key,
    rendering::images::image_buffer_view target)
  {
    auto rec = m_sized_fonts.at(key.font_index()).lock();
    J_ASSERT_NOT_NULL(rec);
    auto font = rec->face();
    J_ASSERT_NOT_NULL(font);
    J_ASSERT(FT_Activate_Size(rec->size) == 0, "FT_Activate_Size failed.");
    rasterize_glyph_impl(font, key.glyph_index(), target.data, target.info.stride());
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<ft_rasterizer> def(
      "fonts.rasterizer.freetype",
      "FreeType font rasterizer",
      s::implements = s::interface<font_rasterizer>);
  }
}
