#include "rendering/vulkan/images/image.hpp"

#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/images/image_resource.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/images/image_view_description.hpp"
#include "rendering/vulkan/images/image_description.hpp"

namespace j::rendering::vulkan::images {
  namespace i = j::rendering::images;
  namespace {
    VkFormat convert_image_format(i::image_format format) noexcept {
      switch (format) {
      case i::image_format::gray8: return VK_FORMAT_R8_UNORM;
      case i::image_format::rgb24: return VK_FORMAT_R8G8B8_UNORM;
      case i::image_format::bgr24: return VK_FORMAT_B8G8R8_UNORM;
      case i::image_format::rgba32: return VK_FORMAT_R8G8B8A8_UNORM;
      case i::image_format::bgra32: return VK_FORMAT_B8G8R8A8_UNORM;
      }
    }
  }
  image::image(const device_context_base & context, const image_description & desc)
    : size(desc.size),
      vk_format(convert_image_format(desc.format)),
      dimensionality(desc.dimensionality),
      vk_device_memory(VK_NULL_HANDLE),
      device(context.device_ref())
  {
    J_ASSERT_NOT_NULL(size.x, size.y);
    auto vk_device = context.device().vk_device;
    const VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = desc.dimensionality == 2 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D,
      .format = vk_format,
      .extent = {
        .width = size.x,
        .height = size.y,
        .depth = 1,
      },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    J_VK_CALL_CHECKED(vkCreateImage, vk_device, &create_info, nullptr, &vk_image);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vk_device, vk_image, &requirements);

    try {
      u32_t index = context.physical_device().find_memory_type_index(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      const VkMemoryAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = index,
      };
      J_VK_CALL_CHECKED(vkAllocateMemory, vk_device, &allocate_info, nullptr, &vk_device_memory);
      J_VK_CALL_CHECKED(vkBindImageMemory, vk_device, vk_image, vk_device_memory, 0);
    } catch (...) {
      if (vk_device_memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_device, vk_device_memory, nullptr);
      }
      vkDestroyImage(vk_device, vk_image, nullptr);
      throw;
    }
    context.set_object_name(vk_image, desc.name);
  }

  const image_view_ref & image::default_image_view_ref() {
    if (!m_default_image_view)  {
      m_default_image_view = image_view_ref::create(image_ref::from_this(this), false);
    }
    return m_default_image_view;
  }

  image::~image() {
    if (auto dev = device.lock()) {
      const VkDevice vk_device = dev.get().vk_device;
      vkFreeMemory(vk_device, vk_device_memory, nullptr);
      vkDestroyImage(vk_device, vk_image, nullptr);
    }
  }
  DEFINE_RESOURCE_DEFINITION(image, image_description, image_state)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::images::image, j::rendering::vulkan::images::image_description);
