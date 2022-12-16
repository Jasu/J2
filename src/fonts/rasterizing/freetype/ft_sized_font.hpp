#pragma once

#include "fonts/rasterizing/rasterizer_font.hpp"
#include "mem/shared_holder.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace j::fonts::rasterizing::freetype {
  class ft_font;
  class ft_sized_font final : public rasterizer_font {
  public:
    ft_sized_font(mem::shared_ptr<ft_font> font, math::s16_16 pixel_size, u32_t index);

    J_INLINE_GETTER FT_Face face() const noexcept {
      return size->face;
    }

    strings::const_string_view family_name() const noexcept override;

    strings::const_string_view style_name() const noexcept override;

    ft_sized_font(const ft_sized_font & rhs) noexcept = delete;
    ft_sized_font & operator=(const ft_sized_font & rhs) noexcept = delete;
    ~ft_sized_font();

    FT_Size size = nullptr;
    u32_t index;
    mem::shared_holder font;
  };
}
