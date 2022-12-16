#include "events/handle.hpp"
#include "events/assert.hpp"
#include "events/event_loop.hpp"
#include "events/stream_handle.hpp"
#include "events/prepare_handle.hpp"
#include "events/poll_handle.hpp"
#include "events/signal_handle.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    template<typename T>
    void destruct(T * J_NOT_NULL t) noexcept {
      t->~T();
    }

    void do_destroy(uv_handle_t * J_NOT_NULL t) noexcept {
      auto s = (base_handle*)t->data;
      J_ASSUME(s->state == handle_state::closing);
      s->state = handle_state::closed;
      ((event_loop*)t->loop->data)->on_before_delete(s);
      s->trigger(after_close_event{s});
      s->visit([](auto * handle) noexcept {
        destruct(handle);
      });
      ::j::free(subtract_bytes(s, s->handle_size));
    }
  }

  base_handle::base_handle(handle_type type, u16_t handle_size, i32_t fd) noexcept
    : type(type),
      handle_size(handle_size),
      fd(fd)
  {
    J_ASSUME(type != handle_type::none);
    J_ASSUME(handle_size >= 64U);
  }

  J_RETURNS_NONNULL event_loop * base_handle::get_loop() noexcept {
    return (event_loop*)handle()->loop->data;
  }

  void base_handle::close() noexcept {
    assert_alive();
    trigger(before_close_event{this});
    state = handle_state::closing;
    uv_close(handle(), do_destroy);
  }

  poll_handle * base_handle::as_poll_handle() noexcept {
    J_ASSUME(type == handle_type::poll);
    return static_cast<poll_handle*>(this);
  }

  stream_handle * base_handle::as_stream_handle() noexcept {
    J_ASSUME(type == handle_type::tty);
    return static_cast<stream_handle*>(this);
  }

  signal_handle * base_handle::as_signal_handle() noexcept {
    J_ASSUME(type == handle_type::signal);
    return static_cast<signal_handle*>(this);
  }

  J_RETURNS_NONNULL prepare_handle * base_handle::as_prepare_handle() noexcept {
    J_ASSUME(type == handle_type::prepare);
    return static_cast<prepare_handle*>(this);
  }

  bool base_handle::trigger(const before_close_event & event) noexcept {
    return common_events.trigger(&event, 1U << (u8_t)event_type::before_close);
  }

  bool base_handle::trigger(const after_close_event & event) noexcept {
    return common_events.trigger(&event, 1U << (u8_t)event_type::after_close);
  }

  bool base_handle::trigger(const error_event & event) noexcept {
    return common_events.trigger(&event, 1U << (u8_t)event_type::error);
  }

  bool base_handle::off(const before_close_handler & handler) noexcept {
    return common_events.off(handler);
  }
  bool base_handle::off(const after_close_handler & handler) noexcept {
    return common_events.off(handler);
  }
  bool base_handle::off(const error_handler & handler) noexcept {
    return common_events.off(handler);
  }

  void base_handle::on(const before_close_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, false, 1U << (u8_t)event_type::before_close);
  }
  void base_handle::on(const after_close_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, false, 1U << (u8_t)event_type::after_close);
  }
  void base_handle::on(const error_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, false, 1U << (u8_t)event_type::error);
  }

  void base_handle::once(const before_close_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, true, 1U << (u8_t)event_type::before_close);
  }
  void base_handle::once(const after_close_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, true, 1U << (u8_t)event_type::after_close);
  }
  void base_handle::once(const error_handler & handler) noexcept {
    assert_alive();
    common_events.on(handler, true, 1U << (u8_t)event_type::error);
  }

  namespace detail {
    bool handler_set::trigger(const void * J_NOT_NULL e, u16_t mask) {
      J_ASSUME(!is_active);
      is_active = true;
      for (u32_t i = 0U, sz = handlers.size(); i < sz; ++i) {
        auto & h = handlers[i];
        if (h.is_marked_for_deletion || !(mask & h.mask)) {
          continue;
        }
        if (h.is_one_shot) {
          h.is_marked_for_deletion = true;
        }
        h.handler(e);
      }
      is_active = false;
      /*
        J_DEBUG("Clearing");
        for (auto it = handlers.begin(); it != handlers.end();) {
        J_DEBUG("  Handler: del={} oneshot={}", (bool)it->is_marked_for_deletion, (bool)it->is_one_shot);
        it = it->is_marked_for_deletion ? handlers.erase(it) : it + 1;
        }
        J_DEBUG("Size {} E {}", handlers.size(), handlers.empty());
      */
      return handlers.empty();
    }
    bool handler_set::empty() const noexcept {
      for (auto & h : handlers) {
        if (!h.is_marked_for_deletion) {
          return false;
        }
      }
      return true;
    }

    bool handler_set::off(const event_handler & handler) noexcept {
      bool had = false;
      for (auto it = handlers.begin(); it != handlers.end();) {
        if (it->handler == handler) {
          if (is_active) {
            it->is_marked_for_deletion = true;
            ++it;
          } else {
            it = handlers.erase(it);
          }
          if (had) {
            break;
          }
        } else if (!it->is_marked_for_deletion) {
          had = true;
        }
      }
      return !had;
      J_FAIL("Could not find handler to erase.");
    }

    void handler_set::on(const event_handler & handler, bool is_one_shot, u16_t mask) noexcept {
      handlers.emplace_back(handler, is_one_shot, mask);
    }
  }
}
