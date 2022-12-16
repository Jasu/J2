#include "rendering/vulkan/rendering/renderer.hpp"

#include "files/paths/path.hpp"
#include "containers/obstack.hpp"
#include "containers/trivial_array.hpp"
#include "mem/shared_ptr.hpp"
#include "debug/timing.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/command_buffers/command_buffer.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/exception.hpp"
#include "rendering/vulkan/state/graphviz_dump.hpp"
#include "rendering/vulkan/surfaces/surface.hpp"
#include "rendering/vulkan/rendering/render_frame.hpp"
#include "rendering/vulkan/rendering/reserved_resource.hpp"
#include "rendering/vulkan/rendering/scene.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"

J_DEFINE_TIMERS("Rendering", frame)

namespace {
  const VkPipelineStageFlags wait_stages[]{
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
}

namespace j::rendering::vulkan::rendering {
  renderer::renderer(const mem::shared_ptr<const device_context_base> & context, surfaces::surface & surface)
  : m_render_frames{ containers::uninitialized, 2 },
    m_context(context),
    m_surface_renderer(*m_context, surface)
  {
    m_state_tracker.define_epoch(epochs::render_pass);
    m_state_tracker.define_epoch(epochs::pipeline_binding);
    m_state_tracker.define_epoch(epochs::draw_command);
    for (sz_t i = 0; i < 2; ++i) {
      m_render_frames.initialize_element(*m_context, *this);
      m_render_frames[i].on_become_ready.connect<&renderer::render>(this);
    }
  }

  void renderer::finalize() {
    J_ASSERT_NOT_NULL(m_context);
    for (auto & f : m_render_frames) {
      f.finalize(*m_context);
    }
    m_surface_renderer.finalize(*m_context);
    m_context.reset();
  }

  renderer::~renderer() {
    if (m_context) {
      finalize();
    }
  }

  render_frame * renderer::get_free_render_frame() noexcept {
    for (render_frame & frame : m_render_frames) {
      if (frame.is_ready) {
        return &frame;
      }
    }
    return nullptr;
  }

  void renderer::render() {
    if (J_UNLIKELY(!m_scene)) { // || m_scene->stamp() == m_scene_stamp) {
      return;
    }

    J_START_TIMER(frame);

    while (auto render_frame = get_free_render_frame()) {
      try {
        auto pair = m_surface_renderer.acquire_frame(*m_context, render_frame->wait_semaphore);
        // Mark the frame budy after acquiring, since acquire may throw if the surface is out of date.
        render_frame->mark_as_busy();
        J_ASSERT_NOT_NULL(pair.second);
        render_context rc(m_context, m_surface_renderer.surface(), *pair.second);
        if (J_UNLIKELY(render_frame->scene != m_scene)) {
          render_frame->scene = m_scene;
          render_frame->command_buffer = ::new command_buffers::command_buffer(*m_context);
        } else {
          J_ASSERT_NOT_NULL(render_frame->command_buffer);
          render_frame->command_buffer->reset();
        }

        command_buffers::command_buffer_builder b(rc, &m_state_tracker);
        try {
          b.begin_render_pass(); b.end_render_pass();
          m_scene->build_command_buffer(b);
        } catch (...) {
          state::graphviz_dump("crashdump.dot", b.state_tracker());
          throw;
        }
        if (J_UNLIKELY(m_graphviz_dump_pending)) {
          state::graphviz_dump("dump.dot", b.state_tracker());
          m_graphviz_dump_pending = false;
        }

        auto cmd_buf = render_frame->command_buffer.get();
        J_ASSUME_NOT_NULL(cmd_buf);

        cmd_buf->begin();
        {
          command_buffers::command_context ctx{rc};
          ctx.vk_command_buffer = cmd_buf->vk_command_buffer;
          render_frame->set_reserved_resources(b.state_tracker().execute(ctx));
        }
        cmd_buf->end();

        const VkSubmitInfo submit_info{
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &render_frame->wait_semaphore.vk_semaphore,
          .pWaitDstStageMask = wait_stages,
          .commandBufferCount = 1,
          .pCommandBuffers = &cmd_buf->vk_command_buffer,
          .signalSemaphoreCount = 1,
          .pSignalSemaphores = &render_frame->signal_semaphore.vk_semaphore,
        };
        J_VK_CALL_CHECKED(vkQueueSubmit, m_context->device().vk_graphics_queue, 1,
                          &submit_info, render_frame->signal_fence.vk_fence);

        // Start polling for the submitted queue in the event loop.
        // This does not actually wait.
        render_frame->poll(*m_context);

        m_surface_renderer.present(*m_context, pair.first, render_frame->signal_semaphore);
        m_scene_stamp = m_scene->stamp;

        m_state_tracker.clear();
        J_STOP_TIMER(frame);
        // J_DUMP_TIMERS();
        on_after_render();
        return;
      } catch (const error_surface_out_of_date & e) {
        m_state_tracker.clear();
        J_VK_INFO("Surface out of date - retrying.");
      } catch (...) {
        m_state_tracker.clear();
        on_after_render();
        throw;
      }
    }

  }

  /// Set [scene] as the current scene.
  void renderer::set_scene(mem::shared_ptr<scene> scene) {
    if (J_UNLIKELY(m_scene == scene)) {
      return;
    }
    m_scene = static_cast<mem::shared_ptr<rendering::scene> &&>(scene);
    if (J_LIKELY(m_scene)) {
      m_scene_stamp = m_scene->stamp - 1;
      m_scene->on_invalidate.connect<&renderer::render>(m_on_invalidate_observer, this);
    }
  }
}
