#pragma once

#include <vulkan/vulkan.h>

namespace j::windowing::x11 {
  class window;
}

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::x11 {
  class surface_factory {
  public:
    VkSurfaceKHR create_surface(const device_context_base & context,
                                j::windowing::x11::window & window) const;
  };
}
