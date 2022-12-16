#pragma once

#include "events/events.hpp"
#include "containers/vector.hpp"
#include "hzd/type_list.hpp"

namespace j::events {
  enum class handle_state : u8_t {
    ready,
    running,
    closing,
    closed,
  };

  namespace detail {
    struct event_handler_record {
      J_BOILERPLATE(event_handler_record, CTOR_NE, COPY_DEL, MOVE_NE)

      event_handler_record(const event_handler & h, bool is_one_shot, u16_t mask) noexcept
      : handler(h),
        is_one_shot(is_one_shot),
        mask(mask)
        { }

      event_handler handler;
      bool is_marked_for_deletion = false;
      bool is_one_shot = false;
      u16_t mask = U16_MAX;
    };

    using handlers_t = noncopyable_vector<event_handler_record>;

    J_INLINE_GETTER u16_t get_event_mask(const poll_event & e) noexcept {
      return e.flags.value;
    }

    J_INLINE_GETTER u16_t get_event_mask(const read_event & e) noexcept {
      return e.is_eof() ? 0b10 : 0b01;
    }

    struct handler_set {
      bool is_active = false;
      handlers_t handlers;

      bool empty() const noexcept;

      bool trigger(const void * J_NOT_NULL e, u16_t mask);

      bool off(const event_handler & handler) noexcept;

      void on(const event_handler & handler, bool is_one_shot, u16_t mask) noexcept;
    };
  }

  struct base_handle {
    handle_type type = handle_type::none;
    handle_state state = handle_state::ready;
    u16_t handle_size = 0U;
    union {
      i32_t fd = -1;
      i32_t signum;
    };
    detail::handler_set common_events{};

    base_handle(handle_type type, u16_t handle_size, i32_t fd) noexcept;

    J_RETURNS_NONNULL event_loop * get_loop() noexcept;

    J_INLINE_GETTER_NONNULL uv_handle_t * handle() const noexcept {
      return subtract_bytes<uv_handle_t *>(this, handle_size);
    }

    J_RETURNS_NONNULL poll_handle * as_poll_handle() noexcept;
    J_RETURNS_NONNULL stream_handle * as_stream_handle() noexcept;
    J_RETURNS_NONNULL signal_handle * as_signal_handle() noexcept;
    J_RETURNS_NONNULL prepare_handle * as_prepare_handle() noexcept;

    void close() noexcept;

    template<typename Fn>
    decltype(auto) visit(Fn && fn) {
      switch(type) {
      case handle_type::none:
        J_FAIL("No type");
      case handle_type::tty:
        return static_cast<Fn&&>(fn)(as_stream_handle());
      case handle_type::signal:
        return static_cast<Fn&&>(fn)(as_signal_handle());
      case handle_type::poll:
        return static_cast<Fn&&>(fn)(as_poll_handle());
      case handle_type::prepare:
        return static_cast<Fn&&>(fn)(as_prepare_handle());
      }
    }

    J_INLINE_GETTER bool is_alive() const noexcept {
      return state < handle_state::closing;
    }
    J_INLINE_GETTER bool is_ready() const noexcept {
      return state == handle_state::ready;
    }
    J_INLINE_GETTER bool is_running() const noexcept {
      return state == handle_state::running;
    }
    J_ALWAYS_INLINE_NO_DEBUG void assert_alive() const {
      J_ASSERT(state < handle_state::closing);
    }
    J_ALWAYS_INLINE_NO_DEBUG void assert_ready() const {
      J_ASSERT(state == handle_state::ready);
    }
    J_ALWAYS_INLINE_NO_DEBUG void assert_running() const {
      J_ASSERT(state == handle_state::running);
    }

    bool trigger(const before_close_event & event) noexcept;
    bool trigger(const after_close_event & event) noexcept;
    bool trigger(const error_event & event) noexcept;

    bool off(const before_close_handler & handler) noexcept;
    bool off(const after_close_handler & handler) noexcept;
    bool off(const error_handler & handler) noexcept;

    void on(const before_close_handler & handler) noexcept;
    void on(const after_close_handler & handler) noexcept;
    void on(const error_handler & handler) noexcept;

    void once(const before_close_handler & handler) noexcept;
    void once(const after_close_handler & handler) noexcept;
    void once(const error_handler & handler) noexcept;
  };

  namespace detail {
    template<typename Event, typename = u16_t>
    struct typed_handler_set : handler_set {
      using handler_set::empty;

      J_ALWAYS_INLINE bool trigger(const Event & event) noexcept {
        return handler_set::trigger(&event, U16_MAX);
      }

      J_ALWAYS_INLINE void on(const event_handler_t<Event> & handler) noexcept {
        handler_set::on(handler, false, U16_MAX);
      }
      J_ALWAYS_INLINE void once(const event_handler_t<Event> & handler) noexcept {
        handler_set::on(handler, true, U16_MAX);
      }

      J_ALWAYS_INLINE bool off(const event_handler_t<Event> & handler) noexcept {
        return handler_set::off(handler);
      }
    };

    template<typename Event>
    struct typed_handler_set<Event, decltype(get_event_mask(declref<const Event>()))> : handler_set {
      J_INLINE_GETTER bool empty() const noexcept {
        return handler_set::empty();
      }

      J_ALWAYS_INLINE bool trigger(const Event & event) noexcept {
        return handler_set::trigger(&event, get_event_mask(event));
      }

      J_ALWAYS_INLINE void on(const event_handler_t<Event> & handler) noexcept {
        handler_set::on(handler, false, U16_MAX);
      }
      J_ALWAYS_INLINE void on(const event_handler_t<Event> & handler, u16_t mask) noexcept {
        handler_set::on(handler, false, mask);
      }

      J_ALWAYS_INLINE void once(const event_handler_t<Event> & handler) noexcept {
        handler_set::on(handler, true, U16_MAX);
      }

      J_ALWAYS_INLINE void once(const event_handler_t<Event> & handler, u16_t mask) noexcept {
        handler_set::on(handler, true, mask);
      }

      J_ALWAYS_INLINE bool off(const event_handler_t<Event> & handler) noexcept {
        return handler_set::off(handler);
      }
    };

    template<typename... Events>
    struct basic_handle : base_handle, protected typed_handler_set<Events>... {
      J_BOILERPLATE(basic_handle, CTOR_DEL, MOVE_DEL, COPY_DEL)
      using events_t = type_list<Events...>;

      J_ALWAYS_INLINE_NO_DEBUG basic_handle(handle_type type, u16_t handle_size, i32_t fd) noexcept
        : base_handle(type, handle_size, fd)
      { }

      template<typename Ev>
      J_INLINE_GETTER bool has_handlers() const noexcept {
        return !typed_handler_set<Ev>::empty();
      }

      template<typename Ev>
        J_INLINE_GETTER const detail::handlers_t & handlers() const noexcept {
        return typed_handler_set<Ev>::handlers;
      }

      using base_handle::trigger;
      using base_handle::off;
      using base_handle::on;
      using base_handle::once;
      using typed_handler_set<Events>::trigger...;
      using typed_handler_set<Events>::off...;
      using typed_handler_set<Events>::on...;
      using typed_handler_set<Events>::once...;
    };
  }
}
