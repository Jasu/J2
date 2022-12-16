#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_buffer_layout.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    namespace detail {
      template<typename Ptr>
      struct uniform_view final {
        J_INLINE_GETTER constexpr bool empty() const noexcept {
          return !data;
        }

        J_INLINE_GETTER explicit constexpr operator bool() const noexcept {
          return data;
        }

        J_INLINE_GETTER constexpr bool operator!() const noexcept {
          return !data;
        }

        uniform_buffer_layout info;
        Ptr * data = nullptr;
      };
    }
  }

  using uniform_view = detail::uniform_view<u8_t>;
  using const_uniform_view = detail::uniform_view<const u8_t>;
}
