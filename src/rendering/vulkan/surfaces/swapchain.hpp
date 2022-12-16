#pragma once

#include <vulkan/vulkan.h>

#include "containers/trivial_array_fwd.hpp"
#include "signals/signal.hpp"

namespace j::rendering::vulkan {
  struct device;
}

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::synchronization {
  struct semaphore;
}

namespace j::rendering::vulkan::surfaces {
  struct swapchain_image;
}
J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::surfaces::swapchain_image);

namespace j::rendering::vulkan::surfaces {
  struct surface;

  /// Set of images that can be rendered onto and presented on a specific surface.
  class swapchain final {
    friend struct surface;

    /// Recreates the swapchain, e.g. when the window is resized.
    ///
    /// \param update_surface_details If true, update the surface capabilities and formats.
    void recreate(const device_context_base & context,
                  surface & surface,
                  bool update_surface_details);

    void finalize(device & device) noexcept;

    swapchain() noexcept;

    ~swapchain();

  public:
    /// Called when the swapchain has been recreated, after it has been created.
    ///
    /// Swapchain recreation happens e.g. on window resize, when the old swapchain gets outdated.
    /// When swapchain is recreated, all images will be recreated as well. Extent of the surface
    /// might have changed.
    ///
    /// The event is not triggered when the swapchain is created for the first time.
    signals::signal<void (const device_context_base &)> on_after_recreate;

    /// Acquire the next image to draw on.
    ///
    /// May recreate the swapchain and consequently trigger [on_after_recreate].
    ///
    /// \param signal_semaphore Semaphore to be signaled when the image is ready.
    swapchain_image & acquire_image(const device_context_base & context,
                                    surface & surface,
                                    synchronization::semaphore & signal_semaphore);

    /// Present [frame] on the screen after waiting for [wait_semaphore].
    void present(const device_context_base & context,
                 surface & surface,
                 swapchain_image & image,
                 synchronization::semaphore & wait_semaphore);

    trivial_array<swapchain_image> swapchain_images;
    /// Chosen surface format of the swapchain.
    VkSurfaceFormatKHR vk_format;
    VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
    /// Size of the swapchain.
    VkExtent2D vk_extent;
  };
}
