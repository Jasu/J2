#include "rendering/vulkan/images/image_view.hpp"

#include "rendering/vulkan/images/image.hpp"
#include "rendering/vulkan/images/image_view_description.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan::images {
  void image_view::recreate(const device_context_base & context,
                            VkImage vk_image,
                            VkFormat format,
                            u8_t dimensionality)
  {
    J_ASSERT_NOT_NULL(vk_image);
    vulkan::device & dev= context.device();
    auto d = dev.vk_device;
    const VkImageViewCreateInfo image_view_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vk_image,
      .viewType = dimensionality == 2U
        ? VK_IMAGE_VIEW_TYPE_2D
        : VK_IMAGE_VIEW_TYPE_1D,
      .format = format,
      .components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    if (vk_image_view) {
      vkDestroyImageView(d, vk_image_view, nullptr);
      vk_image_view = VK_NULL_HANDLE;
    }
    try {
      J_VK_CALL_CHECKED(vkCreateImageView, d, &image_view_info, nullptr, &vk_image_view);
    } catch (...) {
      vk_image_view = VK_NULL_HANDLE;
      throw;
    }
  }

  image_view::image_view(const device_context_base & context, image_view_description && description)
    : device(context.device_ref())
  {
    if (description.is_image()) {
      image_ref ref = static_cast<image_ref &&>(description.as_image());
      auto & img = context.get(ref);
      recreate(context, img.vk_image, img.vk_format, img.dimensionality);
      if (description.retain) {
        image = static_cast<image_ref &&>(ref);
      }
    } else {
      auto & image = description.as_swapchain_image();
      recreate(context, image.vk_image, image.format);
    }
  }

  image_view::~image_view() {
    if (vk_image_view) {
      if (auto dev= device.lock()) {
        vkDestroyImageView(dev.get().vk_device, vk_image_view, nullptr);
      }
    }
  }
  DEFINE_RESOURCE_DEFINITION(image_view, image_view_description, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::images::image_view, j::rendering::vulkan::images::image_view_description);
