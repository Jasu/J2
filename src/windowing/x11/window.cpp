#include "windowing/x11/window.hpp"
#include "windowing/x11/context.hpp"
#include "windowing/x11/exceptions.hpp"
#include "windowing/x11/atom.hpp"

#include <xcb/xinput.h>

namespace {
  const u32_t event_mask = XCB_EVENT_MASK_EXPOSURE
    | XCB_EVENT_MASK_BUTTON_PRESS
    | XCB_EVENT_MASK_BUTTON_MOTION
    | XCB_EVENT_MASK_POINTER_MOTION
    | XCB_EVENT_MASK_ENTER_WINDOW
    | XCB_EVENT_MASK_LEAVE_WINDOW
    | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
}

namespace j::windowing::x11 {
  window::window(mem::shared_ptr<context> && x11_context, u16_t width, u16_t height)
    : j::windowing::window(width, height),
      xcb_window(0),
      m_x11_context(static_cast<mem::shared_ptr<context> &&>(x11_context))
  {
    J_ASSERT_NOT_NULL(m_x11_context);
    auto c = m_x11_context->xcb_connection;
    xcb_window = xcb_generate_id(c);
    auto cookie = xcb_create_window_checked(
      c,
      XCB_COPY_FROM_PARENT, // Depth
      xcb_window,
      m_x11_context->get_root(),
      0, // X
      0, // Y
      width, // Width
      height, // Height
      0, // Border width
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      m_x11_context->xcb_screen->root_visual,
      XCB_CW_EVENT_MASK,
      &event_mask);
    auto error = xcb_request_check(c, cookie);
    if (error) {
      m_x11_context->throw_exception(error);
    }

    struct {
      xcb_input_event_mask_t header{
        XCB_INPUT_DEVICE_ALL_MASTER,
        1
      };
      u32_t mask = XCB_INPUT_XI_EVENT_MASK_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE;
    } const mask;
    cookie = xcb_input_xi_select_events_checked(c, xcb_window, 1, &mask.header);
    error = xcb_request_check(c, cookie);
    if (error) {
      m_x11_context->throw_exception(error);
    }

    const xcb_atom_t wm_protocols = get_atom(c, icccm_wm_properties::WM_PROTOCOLS),
                     wm_delete_window = get_atom(c, icccm_wm_protocols::WM_DELETE_WINDOW);
    J_ASSERT_NOT_NULL(wm_protocols, wm_delete_window);
    auto c2 = xcb_change_property_checked(c, XCB_PROP_MODE_REPLACE, xcb_window, wm_protocols, 4, 32, 1, &wm_delete_window);
    error = xcb_request_check(c, c2);
    if (error) {
      m_x11_context->throw_exception(error);
    }
  }

  window::~window() {
    J_ASSERT(!xcb_window, "Destructor called for a non-destroyed window.");
    m_x11_context.reset();
  }

  void window::show() {
    m_x11_context->log_debug("Mapping window");
    xcb_map_window(m_x11_context->xcb_connection, xcb_window);
  }

  void window::hide() {
    m_x11_context->log_debug("Unmapping window");
    xcb_unmap_window(m_x11_context->xcb_connection, xcb_window);
  }

  void window::destroy() {
    J_ASSERT(xcb_window, "Destroy for a non-existent window.");
    on_before_destroy();
    m_x11_context->destroy_window(xcb_window);
    on_after_destroy();
    xcb_window = 0;
    m_x11_context.reset();
  }

  void window::handle_expose(const xcb_expose_event_t * e) {
    const geometry::rect_u16 region{e->x, e->y, e->width, e->height};
    m_x11_context->log_debug("Expose {}", region);
    on_expose(region);
  }

  void window::handle_button_press(const xcb_button_press_event_t * e) {
    const geometry::vec2i16 pos(e->event_x, e->event_y);
    m_x11_context->log_debug("Button #{} press at {}", e->detail, pos);
    on_button_press(e->detail, pos);
  }

  void window::handle_button_release(const xcb_button_release_event_t * e) {
    const geometry::vec2i16 pos(e->event_x, e->event_y);
    m_x11_context->log_debug("Button #{} release at {}", e->detail, pos);
    on_button_release(e->detail, pos);
  }

  void window::handle_motion_notify(const xcb_motion_notify_event_t * e) {
    const geometry::vec2i16 pos(e->event_x, e->event_y);
    on_mouse_move(pos);
  }

  void window::handle_enter_notify(const xcb_enter_notify_event_t * e) {
    const geometry::vec2i16 pos(e->event_x, e->event_y);
    m_x11_context->log_debug("Enter: {}", pos);
    on_mouse_enter(pos);
  }

  void window::handle_leave_notify(const xcb_leave_notify_event_t * e) {
    const geometry::vec2i16 pos(e->event_x, e->event_y);
    m_x11_context->log_debug("Leave: {}", pos);
    on_mouse_leave(pos);
  }

  void window::handle_key_press(const xcb_key_press_event_t * e) {
    m_x11_context->log_debug("Key press: {}", e->detail);
  }

  void window::handle_key_release(const xcb_key_release_event_t * e) {
    m_x11_context->log_debug("Key release: {}", e->detail);
  }

  void window::handle_map_notify(const xcb_map_notify_event_t *) {
    m_x11_context->log_debug("Map");
    on_show();
  }

  void window::handle_unmap_notify(const xcb_unmap_notify_event_t *) {
    m_x11_context->log_debug("Unmap");
    on_hide();
  }

  void window::handle_reparent_notify(const xcb_reparent_notify_event_t *) {
    m_x11_context->log_debug("Reparent");
    on_reparent();
  }

  void window::handle_configure_notify(const xcb_configure_notify_event_t * e) {
    const geometry::rect_i16 rect{e->x, e->y, e->width, e->height};
    m_x11_context->log_debug("Configure at {}", rect);
    on_resize(rect);
  }
}
