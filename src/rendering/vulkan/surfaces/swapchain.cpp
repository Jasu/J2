#include "rendering/vulkan/surfaces/swapchain.hpp"
#include "exceptions/backtrace.hpp"

#include "containers/trivial_array.hpp"
#include "rendering/vulkan/surfaces/swapchain_image.hpp"
J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::surfaces::swapchain_image);

#include "rendering/vulkan/surfaces/surface.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/synchronization/semaphore.hpp"
#include "rendering/vulkan/exception.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/images/image_view.hpp"
#include "rendering/vulkan/images/image_view_description.hpp"

namespace j::rendering::vulkan::surfaces {
  void swapchain::recreate(const device_context_base & context,
                           surface & surface,
                           bool update_surface_details)
  {
    J_VK_INFO("Initializing swapchain.");
    device & device = context.device();
    const VkDevice vk_device = device.vk_device;
    try {
      const auto & caps = update_surface_details
        ? surface.update_vk_surface_capabilities(context)
        : surface.vk_surface_capabilities;
      vk_extent = caps.currentExtent;
      if (J_UNLIKELY(caps.maxImageExtent.height == 0 || caps.maxImageExtent.width == 0)) {
        // TODO: Handle this - swapchain cannot be created yet.
        J_VK_WARNING("Maximum size is zero.");
      }
      if (J_UNLIKELY(vk_extent.width == U32_MAX || vk_extent.height == U32_MAX)) {
        // TODO: Allow specifying the preferred size from outside. This case occurs
        // on Wayland, where the surface size determines the window size, not the other
        // way around.
        vk_extent.width = vk_extent.height = 600;
        J_VK_WARNING("Special value of extent size met. Guessing something.");
      } else {
        vk_extent.width = clamp(caps.minImageExtent.width,
                                  vk_extent.width, caps.maxImageExtent.width);
        vk_extent.height = clamp(caps.minImageExtent.height,
                                   vk_extent.height, caps.maxImageExtent.height);
      }
      J_VK_DEBUG("Creating swapchain with dimensions {}x{}.", vk_extent.width, vk_extent.height);

      auto & formats = update_surface_details
        ? surface.update_vk_surface_formats(context)
        : surface.vk_surface_formats;
      bool did_find = false, did_find_fallback = false;
      for (auto & format : formats) {
        // Prefer the UNORM format over the SRGB format.
        // Ref. Graphics API Performance Guide for Intel Processor Graphics Gen9
        //      4.1.2. Render Target Definition and Use
        //      "Avoid using sRGB formats where unnecessary"
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM) {
          vk_format = format;
          did_find_fallback = true;
          // Most devices seem to return SRGB first, and e.g. Intel on Linux returns
          // only that.
          if (J_LIKELY(format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
            did_find = true;
            break;
          }
        }
      }
      J_VK_ASSERT(did_find_fallback, "No VK_FORMAT_B8G8R8A8_UNORM format found.");
      if (!did_find) {
        J_VK_WARNING("Could not find a format with sRGB color space.");
      }

      // VkSwapchainKHR previous_swapchain = vk_swapchain;
      u32_t nuimages = max(caps.minImageCount, 2u);
      if (caps.maxImageCount) {
        nuimages = min(nuimages, caps.maxImageCount);
      }

      const u32_t graphics_index = device.graphics_queue_index,
                  present_index = device.present_queue_index;
      const u32_t queues[] = { graphics_index, present_index };

      if (graphics_index != present_index) {
        J_VK_WARNING("Graphics and present queues are separate.");
      }

      const VkSwapchainCreateInfoKHR swapchain_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface.vk_surface,
        .minImageCount = nuimages,
        .imageFormat = vk_format.format,
        .imageColorSpace = vk_format.colorSpace,
        .imageExtent = vk_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = graphics_index == present_index
          ? VK_SHARING_MODE_EXCLUSIVE
          : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = static_cast<u32_t>(graphics_index == present_index ? 1 : 2),
        .pQueueFamilyIndices = queues,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = vk_swapchain,
      };

      VkSwapchainKHR previous_swapchain = vk_swapchain;
      J_VK_CALL_CHECKED(vkCreateSwapchainKHR, vk_device, &swapchain_info, nullptr, &vk_swapchain);

      if (previous_swapchain) {
        vkDestroySwapchainKHR(vk_device, previous_swapchain, nullptr);
      }

      J_VK_CALL_CHECKED(vkGetSwapchainImagesKHR, vk_device, vk_swapchain, &nuimages, nullptr);
      J_ASSERT_NOT_NULL(nuimages);
      J_VK_DEBUG("Swapchain has {} images.", nuimages);

      VkImage images[nuimages];
      J_VK_CALL_CHECKED(vkGetSwapchainImagesKHR, vk_device, vk_swapchain, &nuimages, images);

      if (J_UNLIKELY(!swapchain_images.empty() && nuimages != swapchain_images.size())) {
        J_VK_ERROR("Number of presentable frames changed from {} to {}.",
                               swapchain_images.size(), nuimages);
        J_VK_THROW("Number of swapchain images changed.");
      } else if (swapchain_images.empty()) {
        // Initial run
        swapchain_images = trivial_array<swapchain_image>(containers::uninitialized, nuimages);
        for (u32_t i = 0; i < nuimages; ++i) {
          auto ref = images::image_view_ref::create(images[i], vk_format.format);
          // Initialize the images to avoid present frames referring to old versions of images.
          context.get(ref);
          swapchain_images.brace_initialize_element(static_cast<images::image_view_ref &&>(ref), i);
        }
      } else {
        // Updating
        for (u32_t i = 0; i < nuimages; ++i) {
          J_ASSERT(swapchain_images[i].image_view.is_initialized(), "Swapchain image must be initialized.");
          context.get(swapchain_images[i].image_view).recreate(context, images[i], vk_format.format);
        }
      }
    } catch (...) {
      swapchain_images.clear();
      if (vk_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
        vk_swapchain = VK_NULL_HANDLE;
      }
      throw;
    }
  }

  swapchain_image & swapchain::acquire_image(const device_context_base & context,
                                             surface & surface,
                                             synchronization::semaphore & signal_semaphore)
  {
    const VkDevice device = context.device().vk_device;
    u32_t current_image_index = U32_MAX;
    const VkResult result = vkAcquireNextImageKHR(device, vk_swapchain, U64_MAX,
                                            signal_semaphore.vk_semaphore,
                                            nullptr, &current_image_index);
    if (J_LIKELY(result == VK_SUCCESS)) {
      J_ASSERT(current_image_index < swapchain_images.size(), "Acquired image out of range.");
      return swapchain_images[current_image_index];
    }
    if (J_UNLIKELY(result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)) {
      J_VK_FAIL_FUNCTION(vkAcquireNextImageKHR, result);
    }

    J_VK_INFO("Recreating framebuffer in acquire.");
    context.device().wait_device_idle();
    recreate(context, surface, true);
    on_after_recreate(context);
    throw error_surface_out_of_date() << exceptions::message("Surface out of date in acquire.")
                                      << exceptions::backtrace(::j::backtrace::get_backtrace(1));
  }

  void swapchain::present(const device_context_base & context,
                          surface & surface,
                          swapchain_image & image,
                          synchronization::semaphore & wait_semaphore)
  {
    const VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &wait_semaphore.vk_semaphore,
      .swapchainCount = 1,
      .pSwapchains = &vk_swapchain,
      .pImageIndices = &image.index,
    };

    const VkResult result = vkQueuePresentKHR(context.device().vk_present_queue, &present_info);
    if (J_LIKELY(result == VK_SUCCESS)) {
      return;
    }
    if (result == VK_SUBOPTIMAL_KHR) {
      J_VK_DEBUG("Suboptimal framebuffer in present. Still presenting.");
      return;
    }
    if (J_UNLIKELY(result != VK_ERROR_OUT_OF_DATE_KHR)) {
      J_VK_FAIL_FUNCTION(vkQueuePresentKHR, result);
    }

    J_VK_DEBUG("Recreating swapchain in present.");
    context.device().wait_device_idle();
    recreate(context, surface, true);
    on_after_recreate(context);
    throw error_surface_out_of_date() << exceptions::message("Surface out of date in present.")
                                      << exceptions::backtrace(::j::backtrace::get_backtrace(1));
  }

  void swapchain::finalize(device & device) noexcept {
    if (vk_swapchain != VK_NULL_HANDLE) {
      device.wait_device_idle();
      vkDestroySwapchainKHR(device.vk_device, vk_swapchain, nullptr);
      vk_swapchain = VK_NULL_HANDLE;
    }
  }

  swapchain::swapchain() noexcept {
  }

  swapchain::~swapchain() {
  }
}
