#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/images/image_view_resource.hpp"
#include "rendering/vulkan/device_resource.hpp"
#include "geometry/vec2.hpp"

namespace j::rendering::vulkan::images {
  struct image_description;

  struct image final {
    image(const device_context_base & context, const image_description & desc);

    ~image();

    const image_view_ref & default_image_view_ref();

    image(const image &) = delete;
    image & operator=(const image &) = delete;

    VkImage vk_image;
    geometry::vec2u16 size;
    VkFormat vk_format;
    u8_t dimensionality = 2U;
    VkDeviceMemory vk_device_memory;
    image_view_ref m_default_image_view;
    weak_device_ref device;
  };
}
