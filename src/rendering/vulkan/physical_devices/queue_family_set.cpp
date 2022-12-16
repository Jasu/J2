#include "rendering/vulkan/physical_devices/queue_family_set.hpp"

#include "rendering/vulkan/utils.hpp"
#include "containers/trivial_array.hpp"
#include "windowing/x11/context.hpp"
#include <xcb/xproto.h>
#include <vulkan/vulkan_xcb.h>

J_DEFINE_EXTERN_TRIVIAL_ARRAY(VkQueueFamilyProperties);

namespace j::rendering::vulkan::physical_devices {
  queue_family_set::queue_family_set(VkPhysicalDevice vk_physical_device)
    : m_vk_physical_device(vk_physical_device)
  {
    J_ASSERT_NOT_NULL(m_vk_physical_device);
    u32_t sz;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &sz, nullptr);
    m_queue_families = trivial_array<VkQueueFamilyProperties>(sz);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &sz, m_queue_families.begin());
  }

  pair<const VkQueueFamilyProperties *, u32_t> queue_family_set::find_queue_with_flags(u32_t flags) const noexcept {
    const u32_t sz = m_queue_families.size();
    for (u32_t idx = 0; idx < sz; ++idx) {
      if ((m_queue_families[idx].queueFlags & flags) == flags) {
        return {&m_queue_families[idx], idx};
      }
    }
    return {nullptr, U32_MAX};
  }

  pair<const VkQueueFamilyProperties *, u32_t> queue_family_set::find_first_graphics_queue() const noexcept {
    return find_queue_with_flags(VK_QUEUE_GRAPHICS_BIT);
  }

  pair<const VkQueueFamilyProperties *, u32_t> queue_family_set::find_first_present_queue_for(::j::windowing::x11::context & context) const noexcept {
    auto conn = context.xcb_connection;
    J_ASSERT_NOT_NULL(conn);
    const auto visual = context.xcb_screen->root_visual;
    const u32_t sz = m_queue_families.size();
    for (u32_t idx = 0; idx < sz; ++idx) {
      // TODO: Invert this relationship, i.e. context interface that checks if it can present
      // on the queue to decouple this from X
      if (VK_TRUE == vkGetPhysicalDeviceXcbPresentationSupportKHR(m_vk_physical_device, idx, conn, visual)) {
        return {&m_queue_families[idx], idx};
      }
    }
    return {nullptr, U32_MAX};
  }
}
