#pragma once

#include "rendering/vertex_data/vertex_binding_info.hpp"

namespace j::rendering::vertex_data {
  class vertex_buffer_info final {
  public:
    J_ALWAYS_INLINE constexpr vertex_buffer_info() noexcept = default;

    J_ALWAYS_INLINE constexpr vertex_buffer_info(const vertex_binding_info * J_NOT_NULL info,
                                                 u32_t num_vertices) noexcept
      : m_info(info),
        m_num_vertices(num_vertices)
    { J_ASSUME(m_num_vertices); }

    constexpr const vertex_binding_info & binding_info() const noexcept {
      J_ASSERT_NOT_NULL(m_info);
      return *m_info;
    }

    J_INLINE_GETTER constexpr u32_t num_vertices() const noexcept {
      J_ASSUME(m_num_vertices);
      return m_num_vertices;
    }

    J_INLINE_GETTER constexpr u32_t stride_bytes() const noexcept {
      J_ASSERT_NOT_NULL(m_info);
      return m_info->stride();
    }

    J_INLINE_GETTER constexpr u32_t size_bytes() const noexcept {
      J_ASSERT_NOT_NULL(m_info);
      return m_info->stride() * m_num_vertices;
    }
  private:
    const vertex_binding_info * m_info = nullptr;
    u32_t m_num_vertices = 0;
  };
}
