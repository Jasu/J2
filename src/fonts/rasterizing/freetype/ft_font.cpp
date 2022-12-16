#include "fonts/rasterizing/freetype/ft_font.hpp"
#include "fonts/rasterizing/freetype/assert.hpp"
#include "containers/hash_map.hpp"
#include "mem/shared_ptr_fwd.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::math::s16_16, u32_t, HASH(j::math::fixed_hash<j::math::s16_16>));

namespace j::fonts::rasterizing::freetype {
  ft_font::ft_font(FT_Face face, mem::shared_ptr<ft_rasterizer> rasterizer, const files::path & path)
    : m_face(face),
      m_path(path),
      m_rasterizer(static_cast<mem::shared_ptr<ft_rasterizer> &&>(rasterizer))
  {
    J_ASSERT_NOT_NULL(m_face, m_rasterizer);
  }

  const u32_t * ft_font::get_sized_font(math::s16_16 pixel_size) const noexcept {
    J_ASSERT(pixel_size > 0, "Tried to get a zero-sized font.");
    return m_sizes.maybe_at(pixel_size);
  }

  void ft_font::add_sized_font(math::s16_16 pixel_size, u32_t index) {
    [[maybe_unused]] auto p = m_sizes.emplace(pixel_size, index);
    J_ASSERT(p.second, "Tried to add a sized font twice.");
  }

  ft_font::~ft_font() {
    J_ASSERT_NOT_NULL(m_face);
    J_FT_CALL_CHECKED(FT_Done_Face, m_face);
  }
}
