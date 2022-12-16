#pragma once

#include "rendering/vulkan/rendering/surface_renderer.hpp"
#include "rendering/vulkan/state/state_tracker.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "signals/signal.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::rendering {
  struct scene;
  struct render_frame;

  /// Renderer that targets a single surface.
  ///
  /// This is the primary interface for rendering on a surface.
  class renderer {
  public:
    /// Creates all necessary structures for rendering on [surface] and registers to the event loop.
    renderer(const mem::shared_ptr<const device_context_base> & context, surfaces::surface & surface);

    void finalize();

    ~renderer();

    /// Renders the current scene if it is set and a frame is available.
    void render();

    /// Set [scene] as the current scene.
    void set_scene(mem::shared_ptr<scene> scene);

    /// Get the Vulkan render context of the renderer.
    const device_context_base & context() const noexcept {
      return *m_context;
    }

    renderer(renderer &&) = delete;
    renderer & operator=(renderer &&) = delete;
    J_ALWAYS_INLINE void invalidate() noexcept {
      --m_scene_stamp;
      render();
    }

    J_ALWAYS_INLINE void queue_graphviz_dump() noexcept
    { m_graphviz_dump_pending = true; }

    /// Triggered after rendering, whether successful or not.
    signals::signal<void ()> on_after_render;
  private:
    trivial_array<render_frame> m_render_frames;
    mem::shared_ptr<scene> m_scene;

    /// Gets a free render frame, or nullptr if none are free.
    ///
    /// Does not update the ready state of the frames, trusts the current status.
    /// The status should be updated by polling the file descriptors of the frames.
    render_frame * get_free_render_frame() noexcept;

    mem::shared_ptr<const device_context_base> m_context;
    surface_renderer m_surface_renderer;
    signals::observer<void ()> m_on_invalidate_observer;
    state::state_tracker m_state_tracker;
    u32_t m_scene_stamp = 0;
    bool m_graphviz_dump_pending = false;
  };
}
