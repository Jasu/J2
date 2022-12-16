#pragma once

#include "hzd/utility.hpp"
#include "containers/span.hpp"
#include "input/modifier.hpp"

extern "C" {
  struct xkb_state;
  struct xkb_keymap;
  struct xcb_xkb_state_notify_event_t;
}

namespace j::windowing::x11 {
  class keyboard_context;

  class keyboard_state final {
  public:
    J_BOILERPLATE(keyboard_state, CTOR_CE, COPY_DEL)

    keyboard_state(keyboard_context & ctx, i32_t device_id);

    keyboard_state(keyboard_state && rhs) noexcept
      : m_device_id(rhs.m_device_id),
        m_state(rhs.m_state),
        m_keymap(rhs.m_keymap),
        m_shift_index(rhs.m_shift_index),
        m_ctrl_index(rhs.m_ctrl_index),
        m_alt_index(rhs.m_alt_index),
        m_super_index(rhs.m_super_index),
        m_alt_gr_index(rhs.m_alt_gr_index),
        m_caps_lock_index(rhs.m_caps_lock_index)
    {
      rhs.m_state = nullptr;
      rhs.m_keymap = nullptr;
    }

    keyboard_state & operator=(keyboard_state && rhs) noexcept {
      if (this != &rhs) {
        if (m_state || m_keymap) {
          release();
        }
        m_device_id = rhs.m_device_id;
        m_state = rhs.m_state;
        m_keymap = rhs.m_keymap;
        m_shift_index = rhs.m_shift_index;
        m_ctrl_index = rhs.m_ctrl_index;
        m_alt_index = rhs.m_alt_index;
        m_super_index = rhs.m_super_index;
        m_alt_gr_index = rhs.m_alt_gr_index;
        m_caps_lock_index = rhs.m_caps_lock_index;
        rhs.m_state = nullptr;
        rhs.m_keymap = nullptr;
      }
      return *this;
    }

    void handle_state_notify(const xcb_xkb_state_notify_event_t * J_NOT_NULL event);

    u32_t keycode_utf8_size(u32_t keycode);
    void get_keycode_utf8(u32_t keycode, char * J_NOT_NULL buf, u32_t sz);

    input::modifier_mask active_modifiers() const;

    span<const u32_t> keycode_keysyms(u32_t keycode);

    void recreate(keyboard_context & ctx);

    ~keyboard_state() {
      if (m_state || m_keymap) {
        release();
      }
    }
  private:
    i32_t m_device_id = -1;
    xkb_state * m_state = nullptr;
    xkb_keymap * m_keymap = nullptr;
    void release() noexcept;
    void ref() noexcept;

    u32_t m_shift_index = U32_MAX;
    u32_t m_ctrl_index = U32_MAX;
    u32_t m_alt_index = U32_MAX;
    u32_t m_super_index = U32_MAX;
    u32_t m_alt_gr_index = U32_MAX;
    u32_t m_caps_lock_index = U32_MAX;
  };
}
