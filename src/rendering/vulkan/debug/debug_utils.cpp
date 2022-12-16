#include "rendering/vulkan/debug/debug_utils.hpp"
#include "exceptions/assert_lite.hpp"
#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/device_context.hpp"
#include "rendering/vulkan/device.hpp"
#include "backtrace/dump_backtrace.hpp"

namespace j::rendering::vulkan::inline debug {
  namespace {
    VkBool32 debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT severity,
      VkDebugUtilsMessageTypeFlagsEXT type,
      const VkDebugUtilsMessengerCallbackDataEXT* data,
      void* user_data)
    {
      reinterpret_cast<debug_utils*>(user_data)->on_debug(severity, type, data);
      return VK_FALSE;
    }
  }

  debug_utils::debug_utils(mem::shared_ptr<instance> inst)
    : m_instance(static_cast<mem::shared_ptr<instance> &&>(inst))
  {
    J_ASSERT_NOT_NULL(m_instance);
    J_VK_INFO("Initializing Vulkan debugging");
    m_vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(
      m_instance->vk_instance(), "vkSetDebugUtilsObjectTagEXT");
    m_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
      m_instance->vk_instance(), "vkSetDebugUtilsObjectNameEXT");
    J_ASSERT_NOT_NULL(m_vkSetDebugUtilsObjectTagEXT, m_vkSetDebugUtilsObjectNameEXT);
    const VkDebugUtilsMessengerCreateInfoEXT messenger_info{
      .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext           = nullptr,
      .flags           = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &debug_callback,
      .pUserData       = this,
    };

    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      m_instance->vk_instance(), "vkCreateDebugUtilsMessengerEXT");
    J_ASSERT_NOT_NULL(fn);
    J_VK_CALL_CHECKED(fn, m_instance->vk_instance(), &messenger_info, nullptr, &m_vk_messenger);
  }

  debug_utils::~debug_utils() {
    auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      m_instance->vk_instance(), "vkDestroyDebugUtilsMessengerEXT");
    J_ASSERT_NOT_NULL(fn);
    fn(m_instance->vk_instance(), m_vk_messenger, nullptr);
  }

  void debug_utils::set_object_name(const device_context_base & ctx,
                                    VkObjectType type, uptr_t handle, const char * name) {
    J_ASSERT_NOT_NULL(handle, name);
    const VkDebugUtilsObjectNameInfoEXT info {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .objectType = type,
      .objectHandle = handle,
      .pObjectName = name,
    };
    m_vkSetDebugUtilsObjectNameEXT(ctx.device().vk_device, &info);
  }

  void debug_utils::on_debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                             VkDebugUtilsMessageTypeFlagsEXT type,
                             const VkDebugUtilsMessengerCallbackDataEXT * data) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      J_VK_DEBUG("{} {}", (VkDebugUtilsMessageTypeFlagBitsEXT)type, data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      J_VK_INFO("{} {}", (VkDebugUtilsMessageTypeFlagBitsEXT)type, data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      J_VK_WARNING("{} {}", (VkDebugUtilsMessageTypeFlagBitsEXT)type, data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    default:
      J_VK_ERROR("{} {}", (VkDebugUtilsMessageTypeFlagBitsEXT)type, data->pMessage);
      J_BACKTRACE();
    }
  }
}
