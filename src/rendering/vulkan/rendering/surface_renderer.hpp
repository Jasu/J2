#pragma once

#include "containers/trivial_array_fwd.hpp"
#include "containers/pair.hpp"
#include "signals/signal.hpp"

namespace j::rendering::vulkan::synchronization {
  struct semaphore;
}
namespace j::rendering::vulkan::inline context {
  class device_context_base;
}
namespace j::rendering::vulkan::surfaces {
  class swapchain;
  struct surface;
}

namespace j::rendering::vulkan::rendering {
  struct framebuffer;

  class surface_renderer {
  public:
    explicit surface_renderer(const device_context_base & context, surfaces::surface & surface);

    void finalize(const device_context_base & context);

    ~surface_renderer();

    pair<i32_t, framebuffer *> acquire_frame(const device_context_base & context,
                                                         synchronization::semaphore & signal_semaphore);

    void present(const device_context_base & context, i32_t frame_index,
                 synchronization::semaphore & wait_semaphore);

    surfaces::surface & surface() const noexcept {
      return *m_surface;
    }

    surface_renderer(const surface_renderer &) = delete;
    surface_renderer & operator=(const surface_renderer &) = delete;

  private:
    /// Called after swapchain is recreated.
    void after_swapchain_recreate(const device_context_base & context);
    signals::observer<void (const device_context_base &)> m_on_after_recreate_observer;

  private:
    trivial_array<framebuffer> m_frames;
    surfaces::surface * m_surface;
  };
}
