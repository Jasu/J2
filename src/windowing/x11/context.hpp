#pragma once

#include "mem/shared_from_this.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "containers/hash_map_fwd.hpp"
#include "events/events.hpp"
#include "strings/formatting/format_value.hpp"
#include "logging/severity.hpp"
#include "windowing/x11/x11_extension.hpp"
#include "windowing/x11/keyboard_context.hpp"

extern "C" {
  typedef u32_t xcb_window_t;
  typedef u32_t xcb_visualid_t;
  typedef u32_t xcb_atom_t;
  typedef struct xcb_connection_t xcb_connection_t;
  typedef struct xcb_errors_context_t xcb_errors_context_t;
  typedef struct xcb_screen_t xcb_screen_t;
}

namespace j::windowing::x11 {
  class window;
}
namespace j::logging {
  class logger;
}
namespace j::strings {
  class string;
}
namespace j::events {
  struct alignas(16) event_loop;
}

J_DECLARE_EXTERN_HASH_MAP(xcb_window_t, j::mem::weak_ptr<j::windowing::x11::window>);

namespace j::windowing::x11 {
  class context final : public mem::enable_shared_from_this<context> {
  public:
    explicit context(mem::shared_ptr<logging::logger> logger,
                     mem::shared_ptr<events::event_loop> event_loop);

    ~context();

    void flush();

    mem::shared_ptr<window> create_window(u16_t width, u16_t height);

    xcb_window_t get_root();

    [[nodiscard]] int get_fd();

    [[noreturn]] void throw_exception(const void * J_NOT_NULL error);

    /// Handles current events.
    void handle_events();

    /// Handles already queued events, if any.
    ///
    /// \note This is called by the Vulkan code when presenting a new frame. This
    ///       is because Mesa listens to the same X11 connection with the "special XGE"
    ///       mechanism, which makes certain (present) events only visible to Mesa in a
    ///       separate queue.
    ///       However, this also queues the normal events (if any), causing events to
    ///       be inserted in the queue without the poll on the X11 FD being triggered.
    void handle_queued_events();

    context & operator=(context &&) = delete;
    context(context &&) = delete;
    void do_log(logging::severity sev, const char * message, u32_t num_params, const strings::formatting::format_value * params);

    void log(logging::severity sev, const char * message);

    template<typename... Params>
    void log(logging::severity sev, const char * message, Params && ... params) {
      strings::formatting::format_value values[] = { strings::formatting::format_value(static_cast<Params &&>(params))... };
      do_log(sev, message, sizeof...(Params), values);
    }

    template<typename... Params>
    J_ALWAYS_INLINE void log_warning(const char * message, Params && ... params) {
      log(logging::severity::warning, message, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_ALWAYS_INLINE void log_error(const char * message, Params && ... params) {
      log(logging::severity::error, message, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_ALWAYS_INLINE void log_info(const char * message, Params && ... params) {
      log(logging::severity::info, message, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_ALWAYS_INLINE void log_notice(const char * message, Params && ... params) {
      log(logging::severity::notice, message, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_ALWAYS_INLINE void log_debug(const char * message, Params && ... params) {
      log(logging::severity::debug, message, static_cast<Params &&>(params)...);
    }

    xcb_connection_t * xcb_connection = nullptr;
    xcb_screen_t * xcb_screen = nullptr;
  private:
    void start_polling();
    void stop_polling();
    void handle_event(void * J_NOT_NULL event);

    mem::shared_ptr<logging::logger> m_logger;
    hash_map<xcb_window_t, mem::weak_ptr<window>> m_windows;
    x11_extension m_xinput;
    keyboard_context m_keyboard_context;
    xcb_errors_context_t *m_xcb_errors = nullptr;
    xcb_atom_t m_wm_protocols = 0;
    xcb_atom_t m_wm_delete_window = 0;

    mem::shared_ptr<events::event_loop> m_event_loop;
    i32_t m_active_pollers = 0;
    /// Recursion lock for handle_events() / handle_queued_events().
    bool m_is_processing_events = false;

    friend class window;

    void destroy_window(xcb_window_t win);
    [[nodiscard]] mem::shared_ptr<window> maybe_get_window(xcb_window_t window);

    /// Handles current events.
    void handle_events_cb(const events::poll_event &);
  };
}
