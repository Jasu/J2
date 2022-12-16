#include "rendering/texture_atlas/texture_atlas.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(u64_t, j::rendering::texture_atlas::texture_record);

namespace j::rendering::texture_atlas {
  texture_record::texture_record(const geometry::rect_u16 & rect,
                                 const geometry::vec2u16 bearing,
                                 u16_t lru_epoch,
                                 u32_t creation_stamp) noexcept
    : rect(rect),
      bearing(bearing),
      lru_epoch(lru_epoch),
      lru_count(8U),
      creation_stamp(creation_stamp)
  {
  }

  void texture_record::update_lru(u16_t current_epoch, u16_t increment) noexcept {
    u32_t epoch_diff = (u16_t)current_epoch - lru_epoch;
    epoch_diff = j::min(lru_count, epoch_diff * epoch_diff - epoch_diff);
    lru_count = lru_count - epoch_diff;
    if (increment) {
      lru_epoch = current_epoch;
      if (lru_count + increment > U16_MAX) {
        lru_count = U16_MAX;
      } else {
        lru_count += increment;
      }
    }
  }

  texture_atlas::texture_atlas(geometry::vec2u16 size) noexcept
    : m_allocator(size.x, size.y)
  { }

  texture_record * texture_atlas::insert(u64_t id,
                                         geometry::vec2u16 size,
                                         const geometry::vec2u16 bearing)
  {
    geometry::rect_u16 rect = m_allocator.allocate(size);
    if (J_UNLIKELY(rect.empty())) {
      return nullptr;
    }
    auto p = m_textures.emplace(id, texture_record(rect, bearing, m_lru_epoch, m_create_stamp++));
    J_ASSERT(p.second, "Tried to insert the same key twice.");
    return &p.first->second;
  }

  bool texture_atlas::contains(u64_t id, bool ping_lru) noexcept {
    auto texture = maybe_at(id);
    if (J_UNLIKELY(!texture)) {
      return false;
    }
    if (ping_lru && texture->lru_epoch != m_lru_epoch) {
      texture->update_lru(m_lru_epoch, 8U);
    }
    return true;
  }

  texture_record & texture_atlas::at(u64_t id) {
    return m_textures.at(id);
  }

  texture_record * texture_atlas::maybe_at(u64_t id) noexcept {
    auto it = m_textures.find(id);
    return J_UNLIKELY(it == m_textures.end()) ? nullptr : &it->second;
  }

  void texture_atlas::erase(u64_t id) {
    auto it = m_textures.find(id);
    J_ASSERT(it != m_textures.end(), "Texture not found in atlas.");
    m_allocator.erase(it->second.rect);
    m_textures.erase(it);
  }

  void texture_atlas::mark_used(u64_t id) {
    m_textures.at(id).update_lru(m_lru_epoch, 8U);
  }

  void texture_atlas::erase() {
    u16_t min_lru = U16_MAX;
    hash_map_iterator<u64_t, texture_record> min_it;
    for (auto it = m_textures.begin(), end = m_textures.end(); it != end; ++it) {
      if (it->second.lru_epoch == m_lru_epoch) {
        continue;
      }
      it->second.update_lru(m_lru_epoch, 0U);
      if (min_lru >= it->second.lru_count) {
        if (it->second.lru_count == 0U) {
          m_allocator.erase(it->second.rect);
          m_textures.erase(it);
          return;
        }
        min_lru = it->second.lru_count;
        min_it = it;
      }
    }
    J_REQUIRE(min_it != m_textures.end(), "Tried to erase from atlas, but no space.");
    m_allocator.erase(min_it->second.rect);
    m_textures.erase(min_it);
  }
}
