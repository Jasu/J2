#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/images/image_resource.hpp"

namespace j::rendering::vulkan::images {
  class image_view_description;

  /// Image view, i.e. an image with information about its format.
  struct image_view {
    image_view(const device_context_base & context, image_view_description && description);

    ~image_view();

    /// Recreate the image view, e.g. after the window has been resized.
    ///
    /// This is used to avoid reallocating [image_view]()s during a resize.
    void recreate(const device_context_base & context,
                  VkImage vk_image,
                  VkFormat format,
                  u8_t dimensionality = 2U);

    image_view(const image_view &) = delete;
    image_view & operator=(const image_view &) = delete;

    VkImageView vk_image_view = VK_NULL_HANDLE;
    /// Only set for non-framebuffer images.
    /// \todo Fix this.
    image_ref image;
    weak_device_ref device;
  };
}
