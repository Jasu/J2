#include "windowing/x11/keyboard_state.hpp"
#include "windowing/x11/context.hpp"
#include "windowing/x11/keyboard_context.hpp"

#include <xkbcommon/xkbcommon-x11.h>

#define explicit explicit_
#include <xcb/xkb.h>
#undef explicit

namespace j::windowing::x11 {
  keyboard_state::keyboard_state(keyboard_context & ctx, i32_t device_id)
    : m_device_id(device_id)
  { recreate(ctx); }

  void keyboard_state::handle_state_notify(const xcb_xkb_state_notify_event_t * J_NOT_NULL event) {
    J_ASSERT_NOT_NULL(m_state, m_keymap);
    // J_DEBUG("Update state #{} {} {} {} {} {} {}", m_device_id,
    //         event->baseMods,
    //         event->latchedMods,
    //         event->lockedMods,
    //         event->baseGroup,
    //         event->latchedGroup,
    //         event->lockedGroup);

    xkb_state_update_mask(m_state,
                          event->baseMods,
                          event->latchedMods,
                          event->lockedMods,
                          event->baseGroup,
                          event->latchedGroup,
                          event->lockedGroup);
  }

  u32_t keyboard_state::keycode_utf8_size(u32_t keycode) {
    return xkb_state_key_get_utf8(m_state, keycode, nullptr, 0);
  }
  void keyboard_state::get_keycode_utf8(u32_t keycode, char * J_NOT_NULL buf, u32_t sz) {
    [[maybe_unused]] const u32_t len = xkb_state_key_get_utf8(m_state, keycode, buf, sz);
    J_ASSERT(len + 1 == sz, "Length mismatch");
  }

  span<const u32_t> keyboard_state::keycode_keysyms(u32_t keycode) {
    const u32_t * syms = nullptr;
    i32_t num = xkb_state_key_get_syms(m_state, keycode, &syms);
    return { syms, num };
  }

  input::modifier_mask keyboard_state::active_modifiers() const {
    input::modifier_mask result;
    if (m_shift_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_shift_index, XKB_STATE_MODS_EFFECTIVE))
    {
      result |= input::modifier::shift;
    }

    if (m_ctrl_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_ctrl_index, XKB_STATE_MODS_EFFECTIVE))
    {
      result |= input::modifier::ctrl;
    }

    if (m_alt_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_alt_index, XKB_STATE_MODS_EFFECTIVE))
    {
      result |= input::modifier::alt;
    }

    if (m_super_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_super_index, XKB_STATE_MODS_EFFECTIVE))
    {
      result |= input::modifier::super;
    }

    if (m_alt_gr_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_alt_gr_index, XKB_STATE_MODS_EFFECTIVE))


    {
      result |= input::modifier::alt_gr;
    }
    if (m_caps_lock_index != XKB_MOD_INVALID &&
        xkb_state_mod_index_is_active(m_state, m_caps_lock_index, XKB_STATE_MODS_EFFECTIVE))
    {
      result |= input::modifier::caps_lock;
    }
    return result;
  }

  void keyboard_state::recreate(keyboard_context & ctx) {
    release();
    J_ASSERT(m_device_id >= 0, "No device ID.");
    auto conn = ctx.x11_context().xcb_connection;
    J_ASSERT_NOT_NULL(conn);
    m_keymap = xkb_x11_keymap_new_from_device(ctx.xkb_context(), conn, m_device_id,
                                              XKB_KEYMAP_COMPILE_NO_FLAGS);
    J_REQUIRE_NOT_NULL(m_keymap);
    m_state = xkb_x11_state_new_from_device(m_keymap, conn, m_device_id);
    J_REQUIRE_NOT_NULL(m_state);

    m_shift_index = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_SHIFT);
    m_ctrl_index = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_CTRL);
    m_alt_index = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_ALT);
    m_super_index = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_LOGO);
    m_alt_gr_index = xkb_keymap_mod_get_index(m_keymap, "Mod5");
    m_caps_lock_index = xkb_keymap_mod_get_index(m_keymap, XKB_MOD_NAME_CAPS);
    // auto num = xkb_keymap_num_mods(m_keymap);
    // for (u32_t i = 0; i < num; ++i) {
    //   J_DEBUG("{}: {}", i, xkb_keymap_mod_get_name(m_keymap, i));
    // }
  }

  void keyboard_state::release() noexcept {
    if (m_state) {
      xkb_state_unref(m_state);
      m_state = nullptr;
    }
    if (m_keymap) {
      xkb_keymap_unref(m_keymap);
      m_keymap = nullptr;
    }
  }

  void keyboard_state::ref() noexcept {
    if (m_state) { xkb_state_ref(m_state); }
    if (m_keymap) { xkb_keymap_ref(m_keymap); }
  }
}
