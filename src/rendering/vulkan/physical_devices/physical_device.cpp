#include "rendering/vulkan/physical_devices/physical_device.hpp"

#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"

extern "C" {
  typedef u32_t xcb_window_t;
  typedef u32_t xcb_visualid_t;
  typedef struct xcb_connection_t xcb_connection_t;
}

#include <vulkan/vulkan_xcb.h>

namespace {
  const VkPhysicalDeviceExternalFenceInfo sync_fd_fence_info {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO,
    .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT,
  };
}

namespace j::rendering::vulkan::physical_devices {
  namespace detail {
    struct physical_device_create_info {
      VkPhysicalDevice vk_physical_device;
      queue_family_set queue_families;
      VkExternalFenceProperties vk_fence_sync_fd_properties;
    };
  }

  namespace {
    detail::physical_device_create_info select_physical_device(
      const device_context_base & context)
    {
      J_VK_DEBUG("Selecting physical device.");
      u32_t num_devices = 0;
      VkInstance vk_instance = context.instance().vk_instance();
      J_VK_CALL_CHECKED(vkEnumeratePhysicalDevices, vk_instance, &num_devices, nullptr);
      VkPhysicalDevice devices[num_devices];
      J_VK_CALL_CHECKED(vkEnumeratePhysicalDevices, vk_instance, &num_devices, devices);

      auto & windowing_context = context.windowing_context();

      for (auto vk_physical_device : devices) {
        queue_family_set queue_families{vk_physical_device};
        if (!queue_families.find_first_graphics_queue().first) {
          J_VK_INFO("Skipping a device due to not supporting graphics.");
          continue;
        }
        if (!queue_families.find_first_present_queue_for(windowing_context).first) {
          J_VK_INFO("Skipping a device due to not supporting the windowing context.");
          continue;
        }

        u32_t num;
        J_VK_CALL_CHECKED(vkEnumerateDeviceExtensionProperties, vk_physical_device, nullptr, &num, nullptr);
        VkExtensionProperties * exts = new VkExtensionProperties[num];
        bool had_all_extensions = true;
        try {
          J_VK_CALL_CHECKED(vkEnumerateDeviceExtensionProperties, vk_physical_device, nullptr, &num, exts);
          for (const char * const * ext = device::required_device_extensions; *ext; ++ext) {
            bool had_extension = false;
            for (u32_t i = 0; i < num; ++i) {
              if (strcmp(exts[i].extensionName, *ext) == 0) {
                had_extension = true;
                break;
              }
            }
            if (!had_extension) {
              J_VK_INFO("Skipping a device due to missing extension support.");
              had_all_extensions = false;
              break;
            }
          }
        } catch (...) {
          delete [] exts;
          throw;
        }
        delete [] exts;
        if (!had_all_extensions) {
          continue;
        }

        VkExternalFenceProperties vk_fence_sync_fd_properties = {
          .sType = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES,
          .pNext = nullptr,
        };
        vkGetPhysicalDeviceExternalFenceProperties(vk_physical_device, &sync_fd_fence_info, &vk_fence_sync_fd_properties);
        if (vk_fence_sync_fd_properties.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) {
          J_VK_DEBUG("Skipping a device does not support exporting fences as FDs.");
        }
        J_VK_INFO("Found matching device.");
        return {
          vk_physical_device,
          static_cast<queue_family_set &&>(queue_families),
          vk_fence_sync_fd_properties,
        };
      }
      J_VK_THROW("Could not find a suitable device.");
    }
  }

  physical_device::physical_device(detail::physical_device_create_info && create_info)
    : m_vk_physical_device(create_info.vk_physical_device),
      m_queue_families(static_cast<queue_family_set &&>(create_info.queue_families))
  {
    J_ASSERT_NOT_NULL(m_vk_physical_device);
    vkGetPhysicalDeviceProperties(m_vk_physical_device, &m_vk_properties);
    vkGetPhysicalDeviceMemoryProperties(m_vk_physical_device, &m_vk_memory_properties);
  }

  physical_device::~physical_device() {
    J_VK_DEBUG("Destroying physical device.");
  }

  physical_device::physical_device(const device_context_base & context)
    : physical_device(select_physical_device(context))
  {
  }

  u32_t physical_device::find_memory_type_index(u32_t type, VkMemoryPropertyFlags property_flags) const {
    for (u32_t i = 0; i < m_vk_memory_properties.memoryTypeCount; ++i) {
      if (!(type & (1 << i))) {
        continue;
      }
      if ((m_vk_memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
        return i;
      }
    }
    J_VK_THROW("Memory type not found.");
  }
  DEFINE_RESOURCE_DEFINITION(physical_device, void, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::physical_devices::physical_device, void);
