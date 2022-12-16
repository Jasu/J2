#pragma once

#include "rendering/vulkan/synchronization/semaphore.hpp"
#include "rendering/vulkan/synchronization/fence.hpp"
#include "containers/obstack_fwd.hpp"
#include "events/events.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "mem/unique_ptr.hpp"
#include "signals/signal.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  struct command_buffer;
}

namespace j::rendering::vulkan::rendering {
  class reserved_resource;
  class renderer;
  struct scene;

  /// Represents a slot allowing rendering to a certain frame.
  ///
  /// By default, there are only two render frames, used in a double-buffered manner.
  /// A render frame is considered ready when it is presented to the device. A render
  /// frame that is not ready cannot be rendered on.
  struct render_frame {
    /// Constructs the render frame in ready state.
    ///
    /// All synchronization primitives will be initialized as well.
    render_frame(const device_context_base & context, renderer & r);

    void finalize(const device_context_base & context);

    /// Marks the frame as being in use.
    J_ALWAYS_INLINE void mark_as_busy() noexcept
    { is_ready = false; }

    void poll(const device_context_base & context);

    /// Waits for the frame to be ready.
    ///
    /// The wait is done with Vulkan primitives, i.e. an external event loop is not used.
    ///
    /// \returns true when the wait finished without a timeout.
    bool wait(u64_t timeout = U64_MAX);

    /// Checks whether the frame has become ready.
    ///
    /// In practice, this means that [signal_fence] has been signaled, and the signal was
    /// handled by an external event loop.
    ///
    /// \returns true if the frame is ready.
    bool check_ready_state();

    /// This is signalled when the frame becomes ready.
    ///
    /// Either [wait] or [check_ready_state] may signal this.
    /// Note that adding listeners or signaling the slot is not thread-safe.
    signals::signal<void ()> on_become_ready;

    void set_reserved_resources(containers::obstack<reserved_resource> && resources);

    ~render_frame();

    /// Semaphore to wait for before starting rendering.
    ///
    /// In practice, this is signaled when an image is available from the presentation layer.
    synchronization::semaphore wait_semaphore;

    /// Semaphore to signal after rendering.
    ///
    /// In practice, this is observed by the presentation engine to present the frame.
    synchronization::semaphore signal_semaphore;

    /// Fence to signal after rendering.
    ///
    /// This lets the CPU-side code know that the frame is ready. The fence can be polled with a
    /// file descriptor.
    synchronization::fence signal_fence;

    mem::unique_ptr<command_buffers::command_buffer> command_buffer;

    /// Current scene for the frame.
    mem::shared_ptr<struct scene> scene;

    /// Resources that should only be freed when the frame becomes ready.
    containers::obstack<reserved_resource> reserved_resources;

    class renderer * renderer = nullptr;

    /// The current assumption about the ready state.
    ///
    /// The initial value is true, since no work has been submitted.
    bool is_ready:1 = true;

    bool is_polling:1 = false;

    void ready();

    void on_poll_event(const events::poll_event &);
  };
}
