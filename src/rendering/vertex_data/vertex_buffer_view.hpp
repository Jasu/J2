#pragma once

#include "rendering/vertex_data/vertex_buffer_info.hpp"

namespace j::rendering::vertex_data {
  namespace detail {
    template<typename Ptr>
    class vertex_buffer_view {
    public:
      J_ALWAYS_INLINE constexpr vertex_buffer_view() noexcept = default;

      J_ALWAYS_INLINE vertex_buffer_view(vertex_buffer_info info, Ptr * J_NOT_NULL data) noexcept
        : m_info(info), m_data(data)
      { }

      J_ALWAYS_INLINE vertex_buffer_view(
        u32_t num_vertices,
        const vertex_binding_info * J_NOT_NULL info,
        Ptr * J_NOT_NULL data) noexcept
        : m_info(info, num_vertices),
          m_data(data)
      { }

      J_INLINE_GETTER operator vertex_buffer_view<const Ptr>() const noexcept {
        return vertex_buffer_view<const Ptr>{m_info, m_data};
      }

      J_INLINE_GETTER constexpr const vertex_buffer_info & info() const noexcept
      { return m_info; }

      J_INLINE_GETTER_NONNULL constexpr Ptr * data() const noexcept {
        J_ASSERT_NOT_NULL(m_data);
        return m_data;
      }

      J_INLINE_GETTER constexpr bool empty() const noexcept
      { return !m_data; }

      J_INLINE_GETTER explicit constexpr operator bool() const noexcept {
        return m_data;
      }

      J_INLINE_GETTER constexpr bool operator!() const noexcept {
        return !m_data;
      }
    private:
      vertex_buffer_info m_info;
      Ptr * m_data = nullptr;
    };
  }

  using vertex_buffer_view = detail::vertex_buffer_view<void>;
  using const_vertex_buffer_view = detail::vertex_buffer_view<const void>;
}
