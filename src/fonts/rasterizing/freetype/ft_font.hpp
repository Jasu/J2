#pragma once

#include "math/fixed_hash.hpp"
#include "containers/hash_map_fwd.hpp"
#include "mem/shared_holder.hpp"
#include "files/paths/path.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

J_DECLARE_EXTERN_HASH_MAP(j::math::s16_16, u32_t, HASH(j::math::fixed_hash<j::math::s16_16>));

namespace j::fonts::rasterizing::freetype {
  class ft_rasterizer;

  class ft_font {
  public:
    ft_font(FT_Face face,
            mem::shared_ptr<ft_rasterizer> rasterizer,
            const files::path & path);

    const u32_t * get_sized_font(math::s16_16 pixel_size) const noexcept;
    void add_sized_font(math::s16_16 pixel_size, u32_t index);

    J_INLINE_GETTER FT_Face face() const noexcept {
      return m_face;
    }

    J_INLINE_GETTER const files::path & path() const noexcept {
      return m_path;
    }

    ft_font(const ft_font & rhs) = delete;

    ~ft_font();
  private:
    FT_Face m_face = nullptr;
    hash_map<j::math::s16_16, u32_t, j::math::fixed_hash<j::math::s16_16>> m_sizes;
    files::path m_path;
    mem::shared_holder m_rasterizer;
  };
}
