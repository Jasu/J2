#pragma once

#include "events/common.hpp"
#include "strings/string_view.hpp"
#include "util/flags.hpp"
#include "mem/memory_region.hpp"

namespace j::events {
  struct base_handle;
  struct poll_handle;
  struct stream_handle;

  enum class event_type : u8_t {
    error,
    poll,
    read,
    write,
    signal,
    prepare,
    before_close,
    after_close,
  };

  struct error_event final {
    constexpr inline static event_type type = event_type::error;

    base_handle * handle;
    J_BOILERPLATE(error_event, CTOR_CE)
    int code = -1;
    strings::const_string_view title;
    strings::const_string_view description;

    explicit error_event(int code) noexcept;
  };

  struct poll_event final {
    constexpr inline static event_type type = event_type::poll;
    poll_handle * handle = nullptr;
    poll_flags flags{};
  };

  struct signal_event final {
    constexpr inline static event_type type = event_type::signal;
    signal_handle * handle = nullptr;
    i32_t signal = -1;
  };

  struct prepare_event final {
    constexpr inline static event_type type = event_type::prepare;
    prepare_handle * handle = nullptr;
  };

  struct write_event final {
    constexpr inline static event_type type = event_type::write;
    stream_handle * handle = nullptr;
  };

  struct read_event final {
    constexpr inline static event_type type = event_type::read;
    stream_handle * handle = nullptr;
    j::mem::const_memory_region buffer;

    J_INLINE_GETTER bool is_eof() const noexcept {
      return buffer.empty();
    }
  };

  struct before_close_event final {
    constexpr inline static event_type type = event_type::before_close;
    base_handle * handle;
  };

  struct after_close_event final {
    constexpr inline static event_type type = event_type::after_close;
    base_handle * handle;
  };

  struct fn_ptr_container final {
    J_BOILERPLATE(fn_ptr_container, CTOR_CE, EQ_CE)

    uptr_t data[sizeof(void (fn_ptr_container::*)()) / sizeof(uptr_t)] = {0};
    template<typename Fn>
    J_ALWAYS_INLINE explicit fn_ptr_container(Fn fn) noexcept {
      ::new (data) Fn(fn);
    }

    template<typename Fn>
    J_INLINE_GETTER Fn as() const noexcept {
      return *reinterpret_cast<const Fn*>(data);
    }
  };

  template<typename Ev, typename Fn> struct event_handler_wrapper;

  template<typename Ev, typename Class>
  struct event_handler_wrapper<Ev, void (Class::*)(const Ev &)> final {
    static void invoke(fn_ptr_container fn, void * J_NOT_NULL o, const void * J_NOT_NULL event) {
      auto fnp = fn.as<void (Class::*)(const Ev &)>();
      (reinterpret_cast<Class*>(o)->*fnp)(*reinterpret_cast<const Ev*>(event));
    }
  };

  template<typename Ev, typename Class>
  struct event_handler_wrapper<Ev, void (Class::*)(const Ev &) const> final {
    static void invoke(fn_ptr_container fn, void * J_NOT_NULL o, const void * J_NOT_NULL event) {
      auto fnp = fn.as<void (Class::*)(const Ev &) const>();
      (reinterpret_cast<Class*>(o)->*fnp)(*reinterpret_cast<const Ev*>(event));
    }
  };

  template<typename Ev>
  struct event_handler_wrapper<Ev, void (*)(const Ev &)> final {
    static void invoke(fn_ptr_container fn, void *, const void * J_NOT_NULL event) {
      auto fnp = fn.as<void (*)(const Ev &)>();
      fnp(*reinterpret_cast<const Ev*>(event));
    }
  };

  template<typename Ev, typename Fn> struct dyn_event_handler_wrapper;

  struct event_handler final {
    J_BOILERPLATE(event_handler, EQ_CE)

    void * object = nullptr;
    fn_ptr_container fn;
    void (*wrapper)(fn_ptr_container, void *, const void *) = nullptr;

    J_ALWAYS_INLINE void operator()(const void * J_NOT_NULL event) const {
      J_ASSUME_NOT_NULL(wrapper);
      wrapper(fn, object, event);
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return wrapper;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !wrapper;
    }
  };

  template<typename Ev>
  struct typed_event_handler final {
    J_BOILERPLATE(typed_event_handler, CTOR_CE, EQ_CE)
    event_handler handler;

    constexpr typed_event_handler(void (*fn)(const Ev &)) noexcept
      : handler{nullptr, fn_ptr_container(fn), &event_handler_wrapper<Ev, void(*)(const Ev &)>::invoke}
    { }

    template<typename Class>
    constexpr typed_event_handler(const Class * o, void (Class::*fn)(const Ev &)) noexcept
      : handler{const_cast<Class*>(o), fn_ptr_container(fn), &event_handler_wrapper<Ev, void (Class::*)(const Ev &)>::invoke}
    { }

    template<typename Class>
    constexpr typed_event_handler(const Class * o, void (Class::*fn)(const Ev &) const) noexcept
      : handler{const_cast<Class*>(o), fn_ptr_container(fn), &event_handler_wrapper<Ev, void (Class::*)(const Ev &) const>::invoke}
    { }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return (bool)handler;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !handler;
    }

    J_INLINE_GETTER operator const event_handler &() const noexcept {
      return handler;
    }

    J_ALWAYS_INLINE void operator()(const Ev & event) const {
      handler.operator()(&event);
    }
  };

  template<typename Ev>
  typed_event_handler(void (*)(const Ev &)) -> typed_event_handler<Ev>;

  template<typename Class, typename Ev>
  typed_event_handler(const Class * c, void (Class::*)(const Ev &)) -> typed_event_handler<Ev>;

  template<typename Class, typename Ev>
  typed_event_handler(const Class * c, void (Class::*)(const Ev &) const) -> typed_event_handler<Ev>;

  template<typename Ev>
  using event_handler_t      = typed_event_handler<Ev>;

  using poll_handler         = event_handler_t<poll_event>;
  using write_handler        = event_handler_t<write_event>;
  using read_handler         = event_handler_t<read_event>;
  using signal_handler       = event_handler_t<signal_event>;
  using prepare_handler      = event_handler_t<prepare_event>;

  using before_close_handler = event_handler_t<before_close_event>;
  using after_close_handler  = event_handler_t<after_close_event>;
  using error_handler        = event_handler_t<error_event>;
}
