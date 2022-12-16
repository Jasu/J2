#include "rendering/vulkan/device.hpp"

#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan {
  const char * const device::required_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_external_fence",
    "VK_KHR_external_fence_fd",
    "VK_KHR_draw_indirect_count",
    "VK_KHR_pipeline_executable_properties",
    nullptr,
  };

  device::device(const device_context_base & context) {
    J_VK_INFO("Initializing logical device");

    float queuePriority = 1.0f;
    auto & queues = context.physical_device().queue_families();
    graphics_queue_index = queues.find_first_graphics_queue().second;
    J_VK_ASSERT(graphics_queue_index != U32_MAX, "Graphics queue not found.");
    present_queue_index = queues.find_first_present_queue_for(context.windowing_context()).second;
    J_VK_ASSERT(present_queue_index != U32_MAX, "Present queue not found.");

    // Queue creation info for the present and graphics queues.
    // If both have the same index, this gets passed with length of one,
    // i.e. only the first array element is used.
    VkDeviceQueueCreateInfo queue_infos[]{
      {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphics_queue_index,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
      },
      {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = present_queue_index,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
      },
    };

    u32_t nuqueues = 1;
    if (present_queue_index != graphics_queue_index) {
      J_VK_NOTICE("Different graphics and present queues");
      nuqueues = 2;
    }
    const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR exec_feature{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR,
      .pipelineExecutableInfo = VK_TRUE,
    };

    VkDeviceCreateInfo device_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &exec_feature,
      .flags = 0,
      .queueCreateInfoCount = nuqueues,
      .pQueueCreateInfos = queue_infos,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = J_ARRAY_SIZE(required_device_extensions) - 1U,
      .ppEnabledExtensionNames = required_device_extensions,
      .pEnabledFeatures = nullptr,
    };

    VkPhysicalDevice vk_physical_device = context.physical_device().vk_physical_device();
    J_ASSERT_NOT_NULL(vk_physical_device);
    J_VK_INFO("Creating device");
    J_VK_CALL_CHECKED(vkCreateDevice, vk_physical_device, &device_info, nullptr, &vk_device);

    this->vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(vk_device, "vkGetFenceFdKHR");
    this->vkCmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndirectCountKHR");
    J_ASSERT_NOT_NULL(this->vkGetFenceFdKHR, this->vkCmdDrawIndirectCountKHR);

    vkGetDeviceQueue(vk_device, graphics_queue_index, 0, &vk_graphics_queue);
    vkGetDeviceQueue(vk_device, present_queue_index, 0, &vk_present_queue);

    try {
      VkCommandPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_index,
      };
      J_VK_CALL_CHECKED(vkCreateCommandPool, vk_device, &pool_info, nullptr, &vk_command_pool);
    } catch (...) {
      vkDestroyDevice(vk_device, nullptr);
      vk_device = VK_NULL_HANDLE, vk_graphics_queue = VK_NULL_HANDLE, vk_present_queue = VK_NULL_HANDLE;
      throw;
    }
    J_VK_INFO("Done creating device");
  }

  device::~device() {
    J_ASSERT_NOT_NULL(vk_device, vk_command_pool);
    J_VK_INFO("Finalizing logical device");
    vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);
    vk_command_pool = VK_NULL_HANDLE;
    vkDestroyDevice(vk_device, nullptr);
    vk_device = VK_NULL_HANDLE, vk_graphics_queue = VK_NULL_HANDLE, vk_present_queue = VK_NULL_HANDLE;
  }

  void device::wait_device_idle() {
    J_ASSERT_NOT_NULL(vk_device);
    J_VK_CALL_CHECKED(vkDeviceWaitIdle, vk_device);
  }

  DEFINE_RESOURCE_DEFINITION(device, void, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::device, void);
DEFINE_WEAK_RESOURCE_TEMPLATES(j::rendering::vulkan::device, void);
