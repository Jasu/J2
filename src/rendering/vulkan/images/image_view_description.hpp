#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/images/image_resource.hpp"
#include "util/tagged_union.hpp"

namespace j::rendering::vulkan::images {
  struct swapchain_image_rec {
    VkImage vk_image;
    VkFormat format;
  };

  class image_view_description {
  public:
  private:
    util::tagged_union<image_ref, swapchain_image_rec> m_data;
  public:
    bool retain = true;
    image_view_description(image_ref image, bool retain = true) noexcept;

    image_view_description(VkImage vk_image, VkFormat format) noexcept;

    image_view_description(const image_view_description & rhs);
    image_view_description(image_view_description && rhs) noexcept;
    image_view_description & operator=(const image_view_description & rhs);
    image_view_description & operator=(image_view_description && rhs) noexcept;

    ~image_view_description();

    bool is_image() const noexcept;

    bool is_swapchain_image() const noexcept;

    const image_ref & as_image() const;

    image_ref & as_image();

    const swapchain_image_rec & as_swapchain_image() const;
  };
}
