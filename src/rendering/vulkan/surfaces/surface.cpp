#include "rendering/vulkan/surfaces/surface.hpp"

#include "containers/trivial_array.hpp"
J_DEFINE_EXTERN_TRIVIAL_ARRAY(VkSurfaceFormatKHR);

#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/utils.hpp"

namespace j::rendering::vulkan::surfaces {
  surface::surface(const device_context_base & context, VkSurfaceKHR vk_surface)
    : vk_surface(vk_surface),
      render_pass(create_tag)
  {
    // Don't assert platform window. It is only kept for reference-counting any way,
    // and it is plausible that some use cases might not want to pass anything.
    J_ASSERT_NOT_NULL(vk_surface);
    try {
      update_vk_surface_capabilities(context);
      update_vk_surface_formats(context);
      swapchain.recreate(context, *this, false);
    } catch (...) {
      vkDestroySurfaceKHR(J_VK_INSTANCE, vk_surface, nullptr);
      throw;
    }
  }

  void surface::finalize(device & d) {
    J_ASSERT(vk_surface, "Surface was finalized twice.");
    swapchain.finalize(d);
    vkDestroySurfaceKHR(J_VK_INSTANCE, vk_surface, nullptr);
    vk_surface = nullptr;
  }

  surface::~surface() {
    J_ASSERT(!vk_surface, "Surface was not finalized.");
  }

  const VkSurfaceCapabilitiesKHR & surface::update_vk_surface_capabilities(
    const device_context_base & context)
  {
    J_VK_CALL_CHECKED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
                      context.physical_device().vk_physical_device(), vk_surface,
                      &vk_surface_capabilities);
    return vk_surface_capabilities;
  }

  const trivial_array<VkSurfaceFormatKHR> & surface::update_vk_surface_formats(
    const device_context_base & context)
  {
    VkPhysicalDevice vk_physical_device = context.physical_device().vk_physical_device();
    u32_t nuformats = 0;
    J_VK_CALL_CHECKED(vkGetPhysicalDeviceSurfaceFormatsKHR, vk_physical_device,
                      vk_surface, &nuformats, nullptr);
    J_ASSERT_NOT_NULL(nuformats);
    trivial_array<VkSurfaceFormatKHR> surface_formats(nuformats);
    J_VK_CALL_CHECKED(vkGetPhysicalDeviceSurfaceFormatsKHR, vk_physical_device,
                      vk_surface, &nuformats, surface_formats.begin());
    vk_surface_formats = static_cast<trivial_array<VkSurfaceFormatKHR> &&>(surface_formats);
    return vk_surface_formats;
  }

  swapchain_image & surface::acquire_image(const device_context_base & context,
                                           synchronization::semaphore & signal_semaphore)
  {
    return swapchain.acquire_image(context, *this, signal_semaphore);
  }

  void surface::present(const device_context_base & context,
                        swapchain_image & image,
                        synchronization::semaphore & wait_semaphore)
  {
    swapchain.present(context, *this, image, wait_semaphore);
  }
}
