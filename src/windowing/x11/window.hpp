#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "windowing/window.hpp"

extern "C" {
  typedef u32_t xcb_window_t;
  typedef struct xcb_configure_notify_event_t xcb_configure_notify_event_t;
  typedef struct xcb_button_press_event_t xcb_button_press_event_t;
  typedef struct xcb_button_press_event_t xcb_button_release_event_t;
  typedef struct xcb_enter_notify_event_t xcb_enter_notify_event_t;
  typedef struct xcb_key_press_event_t xcb_key_release_event_t;
  typedef struct xcb_key_press_event_t xcb_key_press_event_t;
  typedef struct xcb_expose_event_t xcb_expose_event_t;
  typedef struct xcb_enter_notify_event_t xcb_leave_notify_event_t;
  typedef struct xcb_motion_notify_event_t xcb_motion_notify_event_t;
  typedef struct xcb_map_notify_event_t xcb_map_notify_event_t;
  typedef struct xcb_reparent_notify_event_t xcb_reparent_notify_event_t;
  typedef struct xcb_unmap_notify_event_t xcb_unmap_notify_event_t;
}

namespace j::windowing::x11 {
  class context;

  class window final : public j::windowing::window {
  public:
    window(mem::shared_ptr<context> && x11_context, u16_t width, u16_t height);

    ~window();

    void show() override;
    void hide() override;

    void destroy() override;

    signals::signal<void (const geometry::rect_u16 &)> on_expose;
    signals::signal<void ()> on_reparent;

    xcb_window_t xcb_window;
  private:
    void handle_expose(const xcb_expose_event_t * e);
    void handle_button_press(const xcb_button_press_event_t * e);
    void handle_button_release(const xcb_button_release_event_t * e);
    void handle_motion_notify(const xcb_motion_notify_event_t * e);
    void handle_enter_notify(const xcb_enter_notify_event_t * e);
    void handle_leave_notify(const xcb_leave_notify_event_t * e);
    void handle_key_press(const xcb_key_press_event_t * e);
    void handle_key_release(const xcb_key_release_event_t * e);
    void handle_map_notify(const xcb_map_notify_event_t * e);
    void handle_unmap_notify(const xcb_unmap_notify_event_t * e);
    void handle_reparent_notify(const xcb_reparent_notify_event_t * e);
    void handle_configure_notify(const xcb_configure_notify_event_t * e);

    mem::shared_ptr<context> m_x11_context;

    friend class context;
  };
}
