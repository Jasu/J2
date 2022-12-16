#include "rendering/vulkan/x11/surface_factory.hpp"

#include "rendering/vulkan/context/device_context_base.hpp"
extern "C" {
  typedef u32_t xcb_visualid_t;
}
#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "windowing/x11/context.hpp"
#include "windowing/x11/window.hpp"
#include "services/service_definition.hpp"
#include <vulkan/vulkan_xcb.h>


namespace j::rendering::vulkan::x11 {
  VkSurfaceKHR surface_factory::create_surface(const device_context_base & context,
                                               j::windowing::x11::window & window) const
  {
    const VkXcbSurfaceCreateInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .connection = context.windowing_context().xcb_connection,
      .window = window.xcb_window,
    };
    J_ASSERT_NOT_NULL(info.connection, info.window);

    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
    VkInstance vk_instance = context.instance().vk_instance();
    J_VK_CALL_CHECKED(vkCreateXcbSurfaceKHR, vk_instance, &info, nullptr, &vk_surface);
    J_ASSERT_NOT_NULL(vk_surface);

    try {
      VkBool32 result = VK_FALSE;
      J_VK_CALL_CHECKED(vkGetPhysicalDeviceSurfaceSupportKHR,
                        context.physical_device().vk_physical_device(),
                        context.device().present_queue_index,
                        vk_surface, &result);
      J_VK_ASSERT(result == VK_TRUE, "Device does not support present for the surface.");
      return vk_surface;
    } catch(...) {
      vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
      throw;
    }
  }

  namespace {
    J_A(ND, NODESTROY) services::service_definition<surface_factory> surface_factory_definition("vulkan.surface.factory.x11", "Vulkan X11 surface factory");
  }
}
