#include "tty/termpaint_to_keysym.hpp"

#include "containers/hash_map.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>
#include <termpaint_event.h>

J_DEFINE_EXTERN_HASH_MAP(const void *, u32_t);


namespace j::tty {
  namespace {
    atom_to_keysym_t atom_to_keysym{};
    bool is_initialized = false;

  }

  const atom_to_keysym_t & get_termpaint_to_keysym() {
    if (is_initialized) {
      return atom_to_keysym;
    }

#define J_KEY(TP, XKB) { termpaint_input_##TP (), XKB_KEY_##XKB }

    const pair<const void *, u32_t> keys[]{
      J_KEY(enter, Return),
      J_KEY(space, space),
      J_KEY(tab, Tab),
      J_KEY(backspace, BackSpace),
      J_KEY(context_menu, Menu),

      J_KEY(delete, Delete),
      J_KEY(end, End),
      J_KEY(home, Home),
      J_KEY(insert, Insert),
      J_KEY(page_down, Page_Down),
      J_KEY(page_up, Page_Up),

      J_KEY(arrow_down, Down),
      J_KEY(arrow_up, Up),
      J_KEY(arrow_left, Left),
      J_KEY(arrow_right, Right),

      J_KEY(numpad_divide, KP_Divide),
      J_KEY(numpad_multiply, KP_Multiply),
      J_KEY(numpad_subtract, KP_Subtract),
      J_KEY(numpad_add, KP_Add),
      J_KEY(numpad_enter, KP_Enter),
      J_KEY(numpad_decimal, KP_Decimal),

      J_KEY(numpad0, KP_0),
      J_KEY(numpad1, KP_1), J_KEY(numpad2, KP_2), J_KEY(numpad3, KP_3),
      J_KEY(numpad4, KP_4), J_KEY(numpad5, KP_5), J_KEY(numpad6, KP_6),
      J_KEY(numpad7, KP_7), J_KEY(numpad8, KP_8), J_KEY(numpad9, KP_9),

      J_KEY(escape, Escape),

      J_KEY(f1, F1), J_KEY(f2, F2), J_KEY(f3, F3), J_KEY(f4, F4),
      J_KEY(f5, F5), J_KEY(f6, F6), J_KEY(f7, F7), J_KEY(f8, F8),
      J_KEY(f9, F9), J_KEY(f10, F10), J_KEY(f11, F11), J_KEY(f11, F11),
    };

    for (auto & p : keys) {
      atom_to_keysym.emplace(p.first, p.second);
    }

    is_initialized = true;
    return atom_to_keysym;
  }
}
