#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "rendering/vulkan/surfaces/window_factory.hpp"

namespace j::windowing::x11 {
  class context;
}
namespace j::rendering::vulkan::inline context {
  class device_context_base;
}
namespace j::rendering::vulkan::x11 {
  class surface_factory;

  class window_factory final : public surfaces::window_factory {
  public:
    window_factory(mem::shared_ptr<surface_factory> && factory);

    mem::shared_ptr<surfaces::vulkan_window> create_window(
      const context::device_context_base & context,
      u16_t width, u16_t height) const override;

  private:
    mem::shared_ptr<surface_factory> m_surface_factory;
  };
}
