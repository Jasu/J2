#include "rendering/vulkan/rendering/surface_renderer.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/context/device_context.hpp"
#include "rendering/vulkan/surfaces/surface.hpp"
#include "rendering/vulkan/surfaces/swapchain_image.hpp"
#include "rendering/vulkan/utils.hpp"
#include "containers/trivial_array.hpp"

namespace j::rendering::vulkan::rendering {
  surface_renderer::surface_renderer(const device_context_base & context,
                                     surfaces::surface & surface)
    : m_frames(containers::uninitialized, surface.swapchain.swapchain_images.size()),
      m_surface(&surface)
  {
    auto ctx = surface_context(static_cast<const device_context &>(context).as_shared(), *m_surface);
    auto & swapchain = surface.swapchain;
    J_ASSERT(!swapchain.swapchain_images.empty(), "Tried to create a surface renderer with no images.");
    for (auto & img : swapchain.swapchain_images) {
      m_frames.initialize_element(
        ctx,
        swapchain.vk_extent,
        surface.render_pass,
        img.image_view);
    }
    swapchain.on_after_recreate.connect<&surface_renderer::after_swapchain_recreate>(
      m_on_after_recreate_observer, this);
  }

  void surface_renderer::finalize(const device_context_base &) {
    m_frames.clear();
  }

  surface_renderer::~surface_renderer() {
  }

  pair<i32_t, framebuffer *> surface_renderer::acquire_frame(
    const device_context_base & context,
    synchronization::semaphore & signal_semaphore
  ) {
    auto & frame = m_surface->acquire_image(context, signal_semaphore);
    J_ASSERT(frame.index < m_frames.size(), "Frame index out of bounds.");
    return {(i32_t)frame.index, &m_frames[frame.index]};
  }

  void surface_renderer::present(const device_context_base & context, i32_t frame_index,
                                 synchronization::semaphore & wait_semaphore)
  {
    auto & swapchain = m_surface->swapchain;
    J_ASSERT(swapchain.swapchain_images.size() > (u32_t)frame_index, "Frame index out of bounds.");
    m_surface->present(context, swapchain.swapchain_images[frame_index], wait_semaphore);
  }

  void surface_renderer::after_swapchain_recreate(const device_context_base & context) {
    auto & swapchain = m_surface->swapchain;
    if (swapchain.swapchain_images.size() != m_frames.size()) {
      J_VK_THROW("Swapchain frame count changed.");
    }
    auto ctx = surface_context(static_cast<const device_context &>(context).as_shared(), *m_surface);
    for (auto & frame : m_frames) {
      frame.resize(ctx, swapchain.vk_extent);
    }
  }
}
