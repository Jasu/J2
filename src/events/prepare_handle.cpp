#include "events/prepare_handle.hpp"
#include "events/assert.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    void prepare_callback(uv_prepare_t * J_NOT_NULL uv_handle) {
      J_ASSUME_NOT_NULL(uv_handle->data);
      auto handle = reinterpret_cast<base_handle*>(uv_handle->data)->as_prepare_handle();
      handle->trigger(prepare_event{handle});
    }
  }

  void prepare_handle::start() {
    assert_alive();
    J_UV_CALL_CHECKED(uv_prepare_start, (uv_prepare_t*)handle(), &prepare_callback);
    state = handle_state::running;
  }

  void prepare_handle::stop() {
    J_UV_CALL_CHECKED(uv_prepare_stop, (uv_prepare_t*)handle());
    state = handle_state::ready;
  }

  J_RETURNS_NONNULL prepare_handle * prepare_handle::create(uv_loop_t * J_NOT_NULL loop) {
    void * data = j::allocate(sizeof(uv_prepare_t) + sizeof(prepare_handle));
    data = add_bytes(data, sizeof(uv_prepare_t));
    return ::new (data) prepare_handle(loop);
  }

  prepare_handle::prepare_handle(uv_loop_t * J_NOT_NULL loop)
    : basic_handle(handle_type::prepare, sizeof(uv_prepare_t), -1)
  {
    uv_prepare_t * h = (uv_prepare_t*)handle();
    J_UV_CALL_CHECKED(uv_prepare_init, loop, h);
    h->data = (base_handle*)this;
  }

  void prepare_handle::prepare() {
    bool has = has_handlers<prepare_event>();
    if (has && is_ready()) {
      start();
    } else if (!has && is_running()) {
      stop();
    }
  }
}
