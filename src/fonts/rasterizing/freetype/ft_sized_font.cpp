#include "fonts/rasterizing/freetype/ft_sized_font.hpp"

#include "fonts/rasterizing/freetype/assert.hpp"
#include "fonts/rasterizing/freetype/ft_font.hpp"
#include "logging/global.hpp"
#include "mem/shared_ptr_fwd.hpp"

#include FT_SIZES_H
#include FT_TRUETYPE_TABLES_H

namespace j::fonts::rasterizing::freetype {
  ft_sized_font::ft_sized_font(mem::shared_ptr<ft_font> font, math::s16_16 pixel_size, u32_t index)
    : rasterizer_font(pixel_size),
      index(index)
  {
    J_ASSERT_NOT_NULL(font);
    FT_Face face = font->face();
    this->font = static_cast<mem::shared_holder &&>(font.holder);
    J_ASSERT_NOT_NULL(face);
    J_FT_CALL_CHECKED(FT_New_Size, face, &size);
    J_ASSERT_NOT_NULL(size);
    FT_Size_RequestRec request = {
      .type = FT_SIZE_REQUEST_TYPE_NOMINAL,
      .width = 0,
      .height = (u32_t)(pixel_size * 64).round(),
      .horiResolution = 0,
      .vertResolution = 0,
    };

    TT_OS2* os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
    J_REQUIRE_NOT_NULL(os2);
    J_REQUIRE(os2->version != 0xFFFF, "OS/2 table has invalid version.");
    FT_Activate_Size(size);
    J_FT_CALL_CHECKED(FT_Request_Size, face, &request);
    math::s16_16 font_units_x(size->metrics.x_scale >> 6, math::raw_tag),
      font_units_y(size->metrics.y_scale >> 6, math::raw_tag);
    ascent = math::s16_16(size->metrics.ascender << 10, math::raw_tag);
    descent = math::s16_16(size->metrics.descender << 10, math::raw_tag);
    line_gap = font_units_y * os2->sTypoLineGap;
    J_WARNING_IF(line_gap <= 0, "Line gap was not positive.");
    // This is not line height, it is the total height of the font.
    // set_line_height(math::s16_16::from_fixed_point<6>(size->metrics.height));
    line_height = ascent - descent + line_gap;
    J_WARNING_IF(line_height <= 0, "Line height was not positive.");

    underline_position = face->underline_position * font_units_y;
    underline_thickness = face->underline_thickness * font_units_y;
    J_WARNING_IF(underline_thickness <= 0, "Underline thickness was not positive.");

    FT_UInt space_index = FT_Get_Char_Index(face, ' ');
    J_REQUIRE_NOT_NULL(space_index);
    J_FT_CALL_CHECKED(FT_Load_Glyph, face, space_index, FT_LOAD_DEFAULT);
    J_REQUIRE(face->glyph->format != FT_GLYPH_FORMAT_BITMAP, "Space character was a bitmap.");

    space_width = math::s16_16(face->glyph->metrics.horiAdvance << 10, math::raw_tag);
  }

  strings::const_string_view ft_sized_font::family_name() const noexcept {
    J_ASSERT_NOT_NULL(size, size->face);
    return size->face->family_name
      ? strings::const_string_view(size->face->family_name)
      : strings::const_string_view();
  }

  strings::const_string_view ft_sized_font::style_name() const noexcept {
    J_ASSERT_NOT_NULL(size, size->face);
    return size->face->style_name
      ? strings::const_string_view(size->face->style_name)
      : strings::const_string_view();
  }

  ft_sized_font::~ft_sized_font() {
    J_ASSERT_NOT_NULL(size);
    FT_Done_Size(size);
  }
}
