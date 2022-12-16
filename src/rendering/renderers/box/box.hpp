#pragma once

#include "rendering/vulkan/geometry/vertex_buffer_suballocation.hpp"
#include "rendering/renderers/box/instance.hpp"
#include "geometry/perimeter.hpp"

namespace j::rendering::renderers::box {
  namespace r = ::j::rendering;
  namespace g = ::j::geometry;
  namespace v = r::vulkan;
  class box final {
  public:
    J_ALWAYS_INLINE box() noexcept = default;

    box(const geometry::rect_i16 & rectangle,
        const colors::rgba8 & background_color,
        const colors::rgba8 & border_left_color,
        const colors::rgba8 & border_right_color,
        const colors::rgba8 & border_top_color,
        const colors::rgba8 & border_bottom_color,
        const g::perimeter_u8 & corner_radius,
        const g::perimeter_u8 & border_size) noexcept
      : m_instance{
          rectangle,
          background_color,
          border_left_color,
          border_right_color,
          border_top_color,
          border_bottom_color,
          border_size,
          corner_radius
        }
    { }

    void assign_from(const box & rhs) noexcept {
      if (rhs == *this) {
        return;
      }
      m_instance = rhs.m_instance;
      if (J_LIKELY(m_allocation)) {
        m_allocation.userdata() = 0;
      }
    }

    box(const box &) = delete;
    box & operator=(const box &) = delete;
    box(box &&) = delete;
    box & operator=(box &&) = delete;

    J_INLINE_GETTER const j::geometry::rect_i16 & rectangle() const noexcept {
      return m_instance.rectangle_px;
    }

    constexpr bool operator==(const box & rhs) const noexcept {
      return m_instance == rhs.m_instance;
    }
  private:
    instance m_instance;
    mutable v::geometry::vertex_buffer_suballocation m_allocation;
    friend class box_renderer;
    friend class box_instance_data_source_handler;
  };
}
