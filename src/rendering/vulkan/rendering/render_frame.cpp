#include "rendering/vulkan/rendering/render_frame.hpp"
#include "rendering/vulkan/rendering/renderer.hpp"
#include "logging/global.hpp"

#include "rendering/vulkan/rendering/reserved_resource.hpp"
#include "events/event_loop.hpp"
#include "events/poll_handle.hpp"
#include "containers/obstack.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/command_buffers/command_buffer.hpp"

namespace j::rendering::vulkan::rendering {
  render_frame::render_frame(const device_context_base & context,
                             class renderer & renderer)
    : wait_semaphore(context),
      signal_semaphore(context),
      signal_fence(context),
      renderer(&renderer)
  {
  }

  render_frame::~render_frame() {
  }

  void render_frame::ready() {
    if (J_LIKELY(!is_ready)) {
      is_ready = true;
      reserved_resources.clear();
      on_become_ready();
    }
  }

  void render_frame::on_poll_event(const events::poll_event &) {
    J_ASSERT(is_polling, "Received a poll event while not polling.");
    is_polling = false;
    check_ready_state();
  }

  void render_frame::poll(const device_context_base & context) {
    J_ASSERT(!is_polling, "Cannot start polling twice.");
    is_polling = true;
    auto h = context.event_loop().add_poll_handle(
      signal_fence.update_fd(context),
      events::poll_flag::readable);
    h->once(events::poll_handler(this, &render_frame::on_poll_event));
  }

  bool render_frame::wait(u64_t timeout) {
    if (J_UNLIKELY(!signal_fence.wait(renderer->context(), timeout))) {
      J_ERROR("Render frame signal fence wait failed");
      return false;
    }
    ready();
    return true;
  }

  bool render_frame::check_ready_state() {
    if (J_UNLIKELY(!signal_fence.is_signaled(renderer->context()))) {
      J_WARNING("Not signaled");
      return false;
    }
    signal_fence.reset(renderer->context());
    ready();
    return true;
  }

  void render_frame::set_reserved_resources(containers::obstack<reserved_resource> && resources) {
    J_ASSERT(reserved_resources.empty(), "Tried to set reserved resources without freeing the previous ones.");
    reserved_resources = static_cast<containers::obstack<reserved_resource> &&>(resources);
  }

  void render_frame::finalize(const device_context_base & context) {
    if (is_polling) {
      J_ASSERT(signal_fence.fd > 2);
      context.event_loop().remove_poll_handle(signal_fence.fd);
      is_polling = false;
    }
    reserved_resources.clear();
    wait_semaphore.finalize(context);
    signal_semaphore.finalize(context);
    signal_fence.finalize(context);
  }
}
