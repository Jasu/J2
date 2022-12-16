#pragma once

#include "rendering/vulkan/images/image_view_resource.hpp"

namespace j::rendering::vulkan::surfaces {
  struct swapchain_image final {
    images::image_view_ref image_view;

    /// Index of the image in the swap chain.
    u32_t index = U32_MAX;
  };
}
