#pragma once

#include "rendering/images/image_source_key.hpp"
#include "rendering/images/image_buffer_view.hpp"

namespace j::rendering::images {
  class static_image_source final : public const_image_buffer_view {
  public:
    using const_image_buffer_view::const_image_buffer_view;
    using const_image_buffer_view::operator=;

    u32_t ctr = 0;
    void invalidate() noexcept { ++ctr; }

    image_source_key get_key() const noexcept {
      return { 0, (uptr_t)this | ((uptr_t)ctr << 48)};
    }

    explicit operator image_source_key() const noexcept {
      return { 0, (uptr_t)this | ((uptr_t)ctr << 48)};
    }
  };
}
