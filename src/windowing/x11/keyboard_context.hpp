#pragma once

#include "exceptions/assert_lite.hpp"
#include "windowing/x11/keyboard_state.hpp"

extern "C" {
  struct xkb_context;
  struct xcb_xkb_new_keyboard_notify_event_t;
  struct xcb_xkb_map_notify_event_t;
  struct xcb_xkb_state_notify_event_t;
}

namespace j::windowing::x11 {
  class context;

  class keyboard_context final {
  public:
    J_ALWAYS_INLINE keyboard_context(context * J_NOT_NULL ctx) noexcept
      : m_context(ctx)
    { }

    void handle_new_keyboard_notify(const xcb_xkb_new_keyboard_notify_event_t * J_NOT_NULL e);
    void handle_map_notify(const xcb_xkb_map_notify_event_t * J_NOT_NULL e);
    void handle_state_notify(const xcb_xkb_state_notify_event_t * J_NOT_NULL e);

    u32_t keycode_utf8_size(u32_t keycode);
    void get_keycode_utf8(u32_t keycode, char * J_NOT_NULL buf, u32_t sz);

    span<const u32_t> keycode_keysyms(u32_t keycode);

    input::modifier_mask active_modifiers() const;

    void initialize();
    void finalize() noexcept;

    J_INLINE_GETTER context & x11_context() const noexcept
    { return *m_context; }

    J_INLINE_GETTER_NONNULL struct xkb_context * xkb_context() noexcept {
      J_ASSERT_NOT_NULL(m_xkb_context);
      return m_xkb_context;
    }

    J_INLINE_GETTER u8_t first_event() const noexcept
    { return m_xkb_events_base; }

    keyboard_context(const keyboard_context &) = delete;
    keyboard_context(keyboard_context &&) = delete;
    keyboard_context & operator=(const keyboard_context &) = delete;
    keyboard_context & operator=(keyboard_context &&) = delete;
  private:
    context * m_context;
    struct xkb_context * m_xkb_context = nullptr;
    keyboard_state m_core_keyboard_state;
    u8_t m_xkb_events_base = 0U;
    u8_t m_xkb_errors_base = 0U;
  };
}
