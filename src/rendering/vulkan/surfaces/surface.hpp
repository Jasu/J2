#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/surfaces/swapchain.hpp"
#include "rendering/vulkan/rendering/render_pass_resource.hpp"
#include "containers/trivial_array_fwd.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(VkSurfaceFormatKHR);

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::surfaces {
  /// Represents a window on which graphics are rendered.
  ///
  /// A surface contains a swapchain, which contains (usually) multiple images.
  struct surface {
    /// Wrap [surface].
    ///
    /// \param vk_surface The surface to wrap. This is created by platform-specific methods.
    ///                   [surface] assumes the ownership of the [vk_surface] though.
    surface(const device_context_base & context, VkSurfaceKHR vk_surface);

    void finalize(device & context);

    ~surface();

    /// Update surface capabilities from the device.
    ///
    /// \returns Reference to the surface capabilities.
    const VkSurfaceCapabilitiesKHR & update_vk_surface_capabilities(const device_context_base & context);

    /// Update surface formats from the device.
    ///
    /// \returns Reference to the surface formats.
    const trivial_array<VkSurfaceFormatKHR> & update_vk_surface_formats(const device_context_base & context);

    swapchain_image & acquire_image(const device_context_base & context,
                                    synchronization::semaphore & signal_semaphore);

    void present(const device_context_base & context,
                 swapchain_image & image,
                 synchronization::semaphore & wait_semaphore);

    surface(const surface &) = delete;
    surface & operator=(const surface &) = delete;

    /// Swap chain of the surface.
    class swapchain swapchain;
    VkSurfaceKHR vk_surface;
    /// Surface capabilities, as per last update.
    VkSurfaceCapabilitiesKHR vk_surface_capabilities;
    /// Formats that the surface supports, as per last update.
    trivial_array<VkSurfaceFormatKHR> vk_surface_formats;
    rendering::render_pass_ref render_pass;
  };
}
