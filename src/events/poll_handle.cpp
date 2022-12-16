#include "events/poll_handle.hpp"
#include "events/assert.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    J_INLINE_GETTER poll_flags from_uv_poll_mask(int e) noexcept {
      poll_flags result;
      if (e & uv_poll_event::UV_READABLE) {
        result.set(poll_flag::readable);
      }
      if (e & uv_poll_event::UV_WRITABLE) {
        result.set(poll_flag::writable);
      }
      return result;
    }

    J_INLINE_GETTER int to_uv_poll_mask(poll_flags e) noexcept {
      int result = 0;
      if (e.has(poll_flag::readable)) {
        result |= uv_poll_event::UV_READABLE;
      }
      if (e.has(poll_flag::writable)) {
        result |= uv_poll_event::UV_WRITABLE;
      }
      return result;
    }

    void poll_event_callback(uv_poll_t *handle, int status, int events) {
      J_ASSUME_NOT_NULL(handle, handle->data);
      // J_DEBUG("{}{}{}",
      //         (events & UV_READABLE) ? "R" : "-",
      //         (events & UV_WRITABLE) ? "W" : "-",
      //         (events & UV_DISCONNECT) ? "D" : "-");
      auto o = reinterpret_cast<poll_handle*>(handle->data)->as_poll_handle();
      if (J_UNLIKELY(status < 0)) {
        o->had_events = true;
        o->trigger(error_event(status));
      } else {
        o->trigger(poll_event{o, from_uv_poll_mask(events)});
      }
    }
  }

  poll_handle::poll_handle(uv_loop_t * J_NOT_NULL loop, int fd, poll_flags flags)
    : basic_handle(handle_type::poll, sizeof(uv_poll_t), fd),
      flags(flags)
  {
    J_ASSUME(fd >= 0);
    J_ASSERT(flags);
    J_UV_CALL_CHECKED(uv_poll_init, loop, (uv_poll_t*)handle(), fd);
    handle()->data = (base_handle*)this;
  }


  void poll_handle::start() {
    assert_alive();
    J_UV_CALL_CHECKED(uv_poll_start, (uv_poll_t*)handle(), to_uv_poll_mask(flags), &poll_event_callback);
    state = handle_state::running;
  }

  void poll_handle::stop() {
    assert_alive();
    J_UV_CALL_CHECKED(uv_poll_stop, (uv_poll_t*)handle());
    state = handle_state::ready;
  }

  [[nodiscard]] J_RETURNS_NONNULL poll_handle * poll_handle::create(uv_loop_t * J_NOT_NULL loop, int fd, poll_flags flags) {
    return ::new (add_bytes(j::allocate(sizeof(uv_poll_t) + sizeof(poll_handle)), sizeof(uv_poll_t))) poll_handle(loop, fd, flags);
  }

  void poll_handle::prepare() {
    if (!is_alive()) {
      return;
    }
    bool has = has_handlers<poll_event>();
    if (has) {
      start();
    } else if (!has && is_running()) {
      stop();
    }
  }
}
