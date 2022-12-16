#include "events/signal_handle.hpp"
#include "events/assert.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    void signal_callback(uv_signal_t * J_NOT_NULL uv_handle, i32_t signum) {
      J_ASSUME_NOT_NULL(uv_handle->data);
      auto handle = reinterpret_cast<base_handle*>(uv_handle->data)->as_signal_handle();
      handle->trigger(signal_event{handle, signum});
    }
  }

  void signal_handle::start() {
    assert_alive();
    J_UV_CALL_CHECKED(uv_signal_start, (uv_signal_t*)handle(), &signal_callback, signum);
    state = handle_state::running;
  }

  void signal_handle::stop() {
    J_UV_CALL_CHECKED(uv_signal_stop, (uv_signal_t*)handle());
    state = handle_state::ready;
  }

  J_RETURNS_NONNULL signal_handle * signal_handle::create(uv_loop_t * J_NOT_NULL loop, int signum) {
    void * data = j::allocate(sizeof(uv_signal_t) + sizeof(signal_handle));
    data = add_bytes(data, sizeof(uv_signal_t));
    return ::new (data) signal_handle(loop, signum);
  }

  signal_handle::signal_handle(uv_loop_t * J_NOT_NULL loop, int signum)
    : basic_handle(handle_type::signal, sizeof(uv_signal_t), signum)
  {
    J_ASSUME(signum >= 0);
    uv_signal_t * h = (uv_signal_t*)handle();
    J_UV_CALL_CHECKED(uv_signal_init, loop, h);
    h->data = (base_handle*)this;
  }

  void signal_handle::prepare() {
    bool has = has_handlers<signal_event>();
    if (has && is_ready()) {
      start();
    } else if (!has && is_running()) {
      stop();
    }
  }
}
