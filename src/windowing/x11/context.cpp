#include "windowing/x11/context.hpp"

#include "containers/hash_map.hpp"
#include "events/event_loop.hpp"
#include "events/poll_handle.hpp"
#include "mem/weak_ptr.hpp"
#include "hzd/mem.hpp"
#include "windowing/x11/atom.hpp"

J_DEFINE_EXTERN_HASH_MAP(xcb_window_t, j::mem::weak_ptr<::j::windowing::x11::window>);

#include "windowing/x11/exceptions.hpp"
#include "windowing/x11/window.hpp"
#include "services/service_definition.hpp"
#include "logging/logging.hpp"
#include "input/key_event.hpp"

#include <xcb/xcb_errors.h>
#include <xcb/xinput.h>
#define explicit explicit_
#include <xcb/xkb.h>
#undef explicit

namespace j::windowing::x11 {
  namespace detail {
    namespace {
      const char * const g_predefined_atom_names[num_predefined_atoms]{
        "WM_CLASS",
        "WM_CLIENT_MACHINE",
        "WM_COLORMAP_WINDOWS",
        "WM_HINTS",
        "WM_ICON_NAME",
        "WM_ICON_SIZE",
        "WM_NAME",
        "WM_NORMAL_HINTS",
        "WM_PROTOCOLS",
        "WM_STATE",
        "WM_TRANSIENT_FOR",

        "WM_CLIENT_LEADER",
        "WM_COMMAND",
        "SM_CLIENT_ID",

        "WM_TAKE_FOCUS",
        "WM_SAVE_YOURSELF",
        "WM_DELETE_WINDOW",

        "WM_CHANGE_STATE",

        "_NET_CLOSE_WINDOW",
        "_NET_MOVERESIZE_WINDOW",
        "_NET_WM_MOVERESIZE",
        "_NET_RESTACK_WINDOW",
        "_NET_REQUEST_FRAME_EXTENTS",

        "_NET_WM_NAME",
        "_NET_WM_VISIBLE_NAME",
        "_NET_WM_ICON_NAME",
        "_NET_WM_VISIBLE_ICON_NAME",
        "_NET_WM_DESKTOP",
        "_NET_WM_WINDOW_TYPE",
        "_NET_WM_STATE",
        "_NET_WM_ALLOWED_ACTIONS",
        "_NET_WM_STRUT",
        "_NET_WM_STRUT_PARTIAL",
        "_NET_WM_ICON_GEOMETRY",
        "_NET_WM_ICON",
        "_NET_WM_PID",
        "_NET_WM_HANDLED_ICONS",
        "_NET_WM_USER_TIME",
        "_NET_FRAME_EXTENTS",

        "_NET_WM_PING",
        "_NET_WM_SYNC_REQUEST",
      };

      xcb_atom_t g_predefined_atom_values[num_predefined_atoms] = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-designator"
        [(u32_t)icccm_wm_properties::WM_CLASS] = XCB_ATOM_WM_CLASS,
        [(u32_t)icccm_wm_properties::WM_CLIENT_MACHINE] = XCB_ATOM_WM_CLIENT_MACHINE,
        [(u32_t)icccm_wm_properties::WM_HINTS] = XCB_ATOM_WM_HINTS,
        [(u32_t)icccm_wm_properties::WM_NAME] = XCB_ATOM_WM_NAME,
        [(u32_t)icccm_wm_properties::WM_ICON_NAME] = XCB_ATOM_WM_ICON_NAME,
        [(u32_t)icccm_wm_properties::WM_ICON_SIZE] = XCB_ATOM_WM_ICON_SIZE,
        [(u32_t)icccm_wm_properties::WM_NORMAL_HINTS] = XCB_ATOM_WM_NORMAL_HINTS,
        [(u32_t)icccm_wm_properties::WM_TRANSIENT_FOR] = XCB_ATOM_WM_TRANSIENT_FOR,

        [(u32_t)icccm_session_properties::WM_COMMAND] = XCB_ATOM_WM_COMMAND,
#pragma clang diagnostic pop
      };
    }

    xcb_atom_t get_atom(xcb_connection_t * c, u32_t index) {
      J_ASSERT_NOT_NULL(c);
      J_ASSERT(index < num_predefined_atoms, "Atom index out of range.");
      if (J_UNLIKELY(!g_predefined_atom_values[index])) {
        const char * const name = g_predefined_atom_names[index];
        xcb_intern_atom_cookie_t request = xcb_intern_atom(c, 0, ::j::strlen(name), name);
        xcb_intern_atom_reply_t * reply = xcb_intern_atom_reply(c, request, nullptr);
        J_ASSERT_NOT_NULL(reply);
        g_predefined_atom_values[index] = reply->atom;
        ::j::free(reply);
      }
      return g_predefined_atom_values[index];
    }
  }
  x11_extension::x11_extension(context & ctx,
                               xcb_extension_t * J_NOT_NULL ext) {
    auto reply = xcb_get_extension_data(ctx.xcb_connection, ext);
    J_ASSERT_NOT_NULL(reply);
    if (reply->present) {
      opcode = reply->major_opcode;
      first_event = reply->first_event;
      first_error = reply->first_error;
    }
  }
  context::context(mem::shared_ptr<logging::logger> logger,
                   mem::shared_ptr<events::event_loop> event_loop)
    : xcb_screen(nullptr),
      m_logger(static_cast<mem::shared_ptr<logging::logger> &&>(logger)),
      m_keyboard_context(this),
      m_xcb_errors(nullptr),
      m_event_loop(static_cast<mem::shared_ptr<events::event_loop> &&>(event_loop))
  {
    J_ASSERT_NOT_NULL(m_logger, m_event_loop);

    log_debug("Initializing X11 context");
    xcb_connection = xcb_connect(nullptr, nullptr);
    J_REQUIRE_NOT_NULL(xcb_connection);
    try {
      J_REQUIRE(!xcb_errors_context_new(xcb_connection, &m_xcb_errors),
                "Failed to initialize X11 errors context.");

      const xcb_setup_t *setup = xcb_get_setup(xcb_connection);
      J_ASSERT_NOT_NULL(setup);
      int num_roots = xcb_setup_roots_length(setup);
      J_REQUIRE(num_roots > 0, "X11 connection does not have any roots.");
      xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
      xcb_screen = it.data;

      m_wm_protocols = get_atom(xcb_connection, icccm_wm_properties::WM_PROTOCOLS);
      m_wm_delete_window = get_atom(xcb_connection, icccm_wm_protocols::WM_DELETE_WINDOW);
      J_ASSERT_NOT_NULL(xcb_screen, m_wm_protocols, m_wm_delete_window);
      m_xinput = x11_extension(*this, &xcb_input_id);

      m_keyboard_context.initialize();

      auto xkb_events = XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY
        | XCB_XKB_EVENT_TYPE_MAP_NOTIFY
        | XCB_XKB_EVENT_TYPE_STATE_NOTIFY;

      xcb_xkb_select_events(xcb_connection,
                            XCB_XKB_ID_USE_CORE_KBD,
                            xkb_events, 0, xkb_events,
                            0, 0, nullptr);
    } catch (...) {
      if (m_xcb_errors) {
        xcb_errors_context_free(m_xcb_errors);
        m_xcb_errors = nullptr;
      }
      if (xcb_connection) {
        xcb_disconnect(xcb_connection);
        xcb_connection = nullptr;
      }
      throw;
    }
  }

  context::~context() {
    log_debug("Finalizing X11 context");
    m_keyboard_context.finalize();
    for (auto & win : m_windows) {
      if (auto w = win.second.lock()) {
        w->destroy();
      }
    }
    if (m_xcb_errors) {
      xcb_errors_context_free(m_xcb_errors);
      m_xcb_errors = nullptr;
    }
    if (xcb_connection) {
      xcb_disconnect(xcb_connection);
      xcb_connection = nullptr;
    }
    xcb_screen = nullptr;
  }

  mem::shared_ptr<window> context::create_window(u16_t width, u16_t height) {
    log_debug("Creating window");
    start_polling();
    auto win = mem::make_shared<window>(shared_from_this(), width, height);
    m_windows.emplace(win->xcb_window, win);
    return win;
  }

  void context::destroy_window(xcb_window_t win) {
    log_debug("Destroying window");
    m_windows.erase(m_windows.find(win));
    xcb_destroy_window(xcb_connection, win);
    stop_polling();
  }

  xcb_window_t context::get_root() {
    J_REQUIRE_NOT_NULL(xcb_screen, xcb_screen->root);
    return xcb_screen->root;
  }

  int context::get_fd() {
    return xcb_get_file_descriptor(xcb_connection);
  }

  void context::flush() {
    log_debug("Flushing connection");
    J_REQUIRE(xcb_flush(xcb_connection) > 0, "X11 Flush failed.");
  }

  mem::shared_ptr<window> context::maybe_get_window(xcb_window_t window) {
    auto it = m_windows.find(window);
    if (J_UNLIKELY(it == m_windows.end())) {
      return {};
    }
    return it->second.lock();
  }

#define J_X11_EVENT_CASE(XCB_EVENT, XCB_EVENT_TYPE, WINDOW_FIELD, FUNCTION_NAME)                \
  case XCB_EVENT:                                                                               \
  if (auto window = maybe_get_window(reinterpret_cast<XCB_EVENT_TYPE*>(event)->WINDOW_FIELD)) { \
    window->FUNCTION_NAME(reinterpret_cast<XCB_EVENT_TYPE*>(event));                            \
  } else {                                                                                      \
    this->log_warning("Received " #XCB_EVENT " for missing window {}.",                         \
                      (u32_t)reinterpret_cast<XCB_EVENT_TYPE*>(event)->WINDOW_FIELD);           \
  }                                                                                             \
  break

  void context::handle_events() {
    J_ASSERT(!m_is_processing_events, "Re-entrant handle_events().");
    m_is_processing_events = true;
    try {
      while (xcb_generic_event_t * event = xcb_poll_for_event(xcb_connection)) {
        handle_event(event);
      }
      m_is_processing_events = false;
    } catch (...) {
      m_is_processing_events = false;
      throw;
    }
  }

  void context::handle_queued_events() {
    if (m_is_processing_events) {
      return;
    }
    m_is_processing_events = true;
    try {
      while (xcb_generic_event_t * event = xcb_poll_for_queued_event(xcb_connection)) {
        handle_event(event);
      }
      m_is_processing_events = false;
    } catch (...) {
      m_is_processing_events = false;
      throw;
    }
  }

  void context::handle_event(void * J_NOT_NULL event_) {
    xcb_generic_event_t * event = (xcb_generic_event_t *)event_;
    try {
      switch (event->response_type & ~0x80) {
        J_X11_EVENT_CASE(XCB_EXPOSE, xcb_expose_event_t, window, handle_expose);
        J_X11_EVENT_CASE(XCB_BUTTON_PRESS, xcb_button_press_event_t, event, handle_button_press);
        J_X11_EVENT_CASE(XCB_BUTTON_RELEASE, xcb_button_release_event_t, event, handle_button_release);
        J_X11_EVENT_CASE(XCB_MOTION_NOTIFY, xcb_motion_notify_event_t, event, handle_motion_notify);
        J_X11_EVENT_CASE(XCB_ENTER_NOTIFY, xcb_enter_notify_event_t, event, handle_enter_notify);
        J_X11_EVENT_CASE(XCB_LEAVE_NOTIFY, xcb_leave_notify_event_t, event, handle_leave_notify);
        J_X11_EVENT_CASE(XCB_KEY_PRESS, xcb_key_press_event_t, event, handle_key_press);
        J_X11_EVENT_CASE(XCB_MAP_NOTIFY, xcb_map_notify_event_t, window, handle_map_notify);
        J_X11_EVENT_CASE(XCB_UNMAP_NOTIFY, xcb_unmap_notify_event_t, window, handle_unmap_notify);
        J_X11_EVENT_CASE(XCB_REPARENT_NOTIFY, xcb_reparent_notify_event_t, window, handle_reparent_notify);
        J_X11_EVENT_CASE(XCB_CONFIGURE_NOTIFY, xcb_configure_notify_event_t, window, handle_configure_notify);
        J_X11_EVENT_CASE(XCB_KEY_RELEASE, xcb_key_release_event_t, event, handle_key_release);
      case XCB_GE_GENERIC: {
        auto e = reinterpret_cast<xcb_ge_generic_event_t*>(event);
        if (e->extension == m_xinput.opcode) {
          switch (e->event_type) {
          case XCB_INPUT_KEY_PRESS:
          case XCB_INPUT_KEY_RELEASE: {
            auto ev = reinterpret_cast<xcb_input_key_press_event_t *>(e);
            if (auto window = maybe_get_window(ev->event)) {
              const i32_t utf8_sz = m_keyboard_context.keycode_utf8_size(ev->detail);
              char buf[utf8_sz + 1U];
              m_keyboard_context.get_keycode_utf8(ev->detail, buf, utf8_sz + 1U);
              const input::key_event result_event{
                ev->detail,
                m_keyboard_context.active_modifiers(),
                (ev->flags & XCB_INPUT_KEY_EVENT_FLAGS_KEY_REPEAT) != 0,
                m_keyboard_context.keycode_keysyms(ev->detail),
                { buf, utf8_sz },
              };
              if (e->event_type == XCB_INPUT_KEY_PRESS) {
                window->on_key_press(result_event);
              } else {
                window->on_key_release(result_event);
              }
            } else {
              this->log_warning("XInput event to an unknown window {}", ev->event);
            }

            break;
          }
          default:
            this->log_warning("Unknown XInput event type {}", e->event_type);
          }
        } else {
          this->log_warning("Unknown generic event extension {}", e->extension);
        }
        break;
      }
      case XCB_CLIENT_MESSAGE: {
        auto msg = reinterpret_cast<xcb_client_message_event_t*>(event);
        if (msg->type != m_wm_protocols) {
          this->log_warning("Unknown client message type {}", msg->type);
          break;
        }
        if (auto window = maybe_get_window(msg->window)) {
          if (msg->data.data32[0] == m_wm_delete_window) {
            window->request_close();
          } else {
            this->log_warning("Unknown WM_PROTOCOLS message type {}", msg->data.data32[0]);
          }
        } else {
          this->log_warning("WM_PROTOCOLS for missing window {}", (u32_t)msg->window);
        }
        break;
      }
      default: {
        if (event->response_type == m_keyboard_context.first_event()) {
          // There's no "base class" for XKB events, so the event type
          // has to be read by questionable means.
          switch (event->pad0) {
          case XCB_XKB_NEW_KEYBOARD_NOTIFY:
            m_keyboard_context.handle_new_keyboard_notify(
              reinterpret_cast<xcb_xkb_new_keyboard_notify_event_t*>(event));
            break;
          case XCB_XKB_MAP_NOTIFY:
            m_keyboard_context.handle_map_notify(
              reinterpret_cast<xcb_xkb_map_notify_event_t*>(event));
            break;
          case XCB_XKB_STATE_NOTIFY:
            m_keyboard_context.handle_state_notify(
              reinterpret_cast<xcb_xkb_state_notify_event_t*>(event));
            break;
          default:
            this->log_warning("Unknown XKB event {}", event->pad0);
            break;
          }
          break;
        }
        this->log_warning("Unknown event {}", event->response_type);
      }
      }
    } catch (...) {
      ::j::free(event);
      throw;
    }
    ::j::free(event);
  }

  void context::handle_events_cb(const events::poll_event &) {
    handle_events();
  }

  void context::do_log(logging::severity sev, const char * message, u32_t num_params, const strings::formatting::format_value * params) {
    m_logger->do_log(sev, true, message, num_params, params);
  }

  void context::log(logging::severity sev, const char * message) {
    m_logger->do_log(sev, true, message, 0, nullptr);
  }

  void context::throw_exception(const void * error_) {
    const xcb_generic_error_t * error = (const xcb_generic_error_t*)error_;
    J_THROW(exception()
            << service_name("X11")
            << major_code(error->major_code)
            << minor_code(error->minor_code)
            << sequence(error->sequence)
            << major_name(xcb_errors_get_name_for_major_code(m_xcb_errors, error->major_code))
            << minor_name(xcb_errors_get_name_for_minor_code(m_xcb_errors, error->major_code, error->minor_code))
            << error_name(xcb_errors_get_name_for_error(m_xcb_errors, error->error_code, nullptr)));
  }

  void context::start_polling() {
    if (!m_active_pollers++) {
      auto h = m_event_loop->add_poll_handle(
        xcb_get_file_descriptor(xcb_connection),
        j::events::poll_flag::readable);
      J_ASSUME_NOT_NULL(h);
      h->on(j::events::poll_handler(this, &context::handle_events_cb));
    }
  }

  void context::stop_polling() {
    J_ASSERT(m_active_pollers, "Tried to stop polling without pollers.");
    if (!--m_active_pollers) {
      flush();
      m_event_loop->remove_poll_handle(xcb_get_file_descriptor(xcb_connection));
    }
  }

  namespace {
    using namespace services;
    J_A(ND, NODESTROY) service_definition<context> x11_context_definition(
      "x11.context",
      "X11 context",
      create = constructor<mem::shared_ptr<logging::logger>, mem::shared_ptr<events::event_loop>>(),
      global_singleton
    );
  }
}
