#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}
namespace j::rendering::vulkan::surfaces {
  struct vulkan_window;
  class window_factory {
  public:
    virtual mem::shared_ptr<surfaces::vulkan_window> create_window(
      const device_context_base & context, u16_t width, u16_t height) const = 0;

    virtual ~window_factory();
  };
}
