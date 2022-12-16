#pragma once

#include "fonts/rasterizing/font_rasterizer.hpp"
#include "files/paths/path_map_fwd.hpp"
#include "containers/hash_map_fwd.hpp"
#include "mem/shared_from_this.hpp"

namespace j::fonts::rasterizing::freetype {
  class ft_sized_font;
  class ft_font;
}

J_DECLARE_EXTERN_HASH_MAP(u16_t, j::mem::weak_ptr<j::fonts::rasterizing::freetype::ft_sized_font>);
J_DECLARE_EXTERN_PATH_MAP(j::mem::weak_ptr<j::fonts::rasterizing::freetype::ft_font>);

namespace j::fonts::rasterizing::freetype {
  class ft_rasterizer final : public font_rasterizer,
                              public mem::enable_shared_from_this<ft_rasterizer> {
  public:
    ft_rasterizer() noexcept;

    void * get_font(const font & f, u32_t dpi) override;

    glyph_info get_glyph_info(const glyph_key & key) override;

    strings::string get_glyph_name(const glyph_key & key) const override;

    mem::shared_ptr<rasterizer_font> get_rasterizer_font(u16_t font_index) const override;

    void rasterize_glyph(const glyph_key & key, rendering::images::image_buffer_view target) override;

    u32_t get_codepoint_glyph_index(const font & f, u32_t codepoint) override;

    ft_rasterizer(const ft_rasterizer &) = delete;

    ~ft_rasterizer();
  private:
    void * m_library;
    files::path_map<mem::weak_ptr<ft_font>> m_fonts;
    hash_map<u16_t, mem::weak_ptr<ft_sized_font>> m_sized_fonts;
    u16_t m_font_index = 0;

    // Accesses m_library
    friend class ft_font;
  };
}
