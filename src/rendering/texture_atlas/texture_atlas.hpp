#pragma once

#include "geometry/rect.hpp"
#include "containers/hash_map_fwd.hpp"
#include "rendering/texture_atlas/row_allocator.hpp"

namespace j::rendering::texture_atlas {
  struct texture_record {
    geometry::rect_u16 rect;
    geometry::vec2u16 bearing;
    u16_t lru_epoch = 0;
    u16_t lru_count = 0;
    u32_t creation_stamp = 0;

    texture_record() noexcept = default;
    explicit texture_record(const geometry::rect_u16 & rect,
                            const geometry::vec2u16 bearing,
                            u16_t lru_epoch,
                            u32_t stamp) noexcept;

    void update_lru(u16_t current_epoch, u16_t increment) noexcept;
  };
}

J_DECLARE_EXTERN_HASH_MAP(u64_t, j::rendering::texture_atlas::texture_record);

namespace j::rendering::texture_atlas {
  struct texture_atlas {
  public:
    texture_atlas(geometry::vec2u16 size) noexcept;

    bool contains(u64_t id, bool ping_lru = false) noexcept;

    texture_record * maybe_at(u64_t id) noexcept;

    J_INLINE_GETTER const texture_record * maybe_at(u64_t id) const noexcept {
      return const_cast<texture_atlas*>(this)->maybe_at(id);
    }

    void tick_lru() noexcept {
      ++m_lru_epoch;
    }

    void mark_used(u64_t id);

    texture_record & at(u64_t id);

    J_ALWAYS_INLINE texture_record & at(u64_t id) const {
      return const_cast<texture_atlas*>(this)->at(id);
    }

    texture_record * insert(u64_t id,
                            geometry::vec2u16 size,
                            const geometry::vec2u16 bearing = geometry::vec2u16{0, 0});

    void erase();

    void erase(u64_t id);

    J_INLINE_GETTER u16_t width() const noexcept {
      return m_allocator.width;
    }

    J_INLINE_GETTER u16_t height() const noexcept {
      return m_allocator.height;
    }
  private:
    hash_map<u64_t, texture_record> m_textures;
    row_allocator m_allocator;
    u16_t m_lru_epoch = 0;
    u32_t m_create_stamp = 0;
  };
}
