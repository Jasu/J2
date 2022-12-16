#pragma once

#include <vulkan/vulkan.h>

#include "mem/shared_ptr_fwd.hpp"

namespace j::rendering::vulkan {
  class instance;
  inline namespace context {class device_context_base; }
}

namespace j::rendering::vulkan::inline debug {
  class debug_utils {
  public:
    debug_utils(mem::shared_ptr<instance> instance);

    ~debug_utils();

    debug_utils(const debug_utils &) = delete;
    debug_utils & operator=(const debug_utils &) = delete;

    void set_object_name(const device_context_base & ctx,
                         VkObjectType type, uptr_t handle, const char * name);

    void on_debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                  VkDebugUtilsMessageTypeFlagsEXT type,
                  const VkDebugUtilsMessengerCallbackDataEXT * data);
  private:
    VkDebugUtilsMessengerEXT m_vk_messenger = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT;
    PFN_vkSetDebugUtilsObjectTagEXT m_vkSetDebugUtilsObjectTagEXT;
    mem::shared_ptr<instance> m_instance;
  };
}
