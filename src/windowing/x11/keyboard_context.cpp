#include "windowing/x11/keyboard_context.hpp"
#include "windowing/x11/context.hpp"
#include <xkbcommon/xkbcommon-x11.h>

namespace j::windowing::x11 {
  void keyboard_context::initialize() {
    J_ASSERT(!m_xkb_context, "Double-initilized XKB context.");
    m_xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    J_REQUIRE(m_xkb_context, "XKB initialization failed.");
    const int success = xkb_x11_setup_xkb_extension(
      m_context->xcb_connection,
      1, 0,
      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
      nullptr, nullptr,
      &m_xkb_events_base,
      &m_xkb_errors_base);
    if (!success) {
      xkb_context_unref(m_xkb_context);
      J_THROW("Setting up XKB with X11 failed.");
    }
    m_core_keyboard_state = keyboard_state(*this, xkb_x11_get_core_keyboard_device_id(m_context->xcb_connection));
  }

  void keyboard_context::finalize() noexcept {
    J_ASSERT_NOT_NULL(m_xkb_context);
    xkb_context_unref(m_xkb_context);
    m_xkb_context = nullptr;
  }

  void keyboard_context::handle_new_keyboard_notify(const xcb_xkb_new_keyboard_notify_event_t * J_NOT_NULL) {
    m_core_keyboard_state.recreate(*this);
  }

  void keyboard_context::handle_map_notify(const xcb_xkb_map_notify_event_t * J_NOT_NULL) {
    m_core_keyboard_state.recreate(*this);
  }

  void keyboard_context::handle_state_notify(const xcb_xkb_state_notify_event_t * J_NOT_NULL e) {
    m_core_keyboard_state.handle_state_notify(e);
  }

  u32_t keyboard_context::keycode_utf8_size(u32_t keycode) {
    return m_core_keyboard_state.keycode_utf8_size(keycode);
  }

  void keyboard_context::get_keycode_utf8(u32_t keycode, char * J_NOT_NULL buf, u32_t sz) {
    m_core_keyboard_state.get_keycode_utf8(keycode, buf, sz);
  }

  span<const u32_t> keyboard_context::keycode_keysyms(u32_t keycode) {
    return m_core_keyboard_state.keycode_keysyms(keycode);
  }

  input::modifier_mask keyboard_context::active_modifiers() const {
    return m_core_keyboard_state.active_modifiers();
  }
}
