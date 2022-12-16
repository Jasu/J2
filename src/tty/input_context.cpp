#include "tty/input_context.hpp"
#include "tty/uv_termpaint_integration.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>

namespace j::tty {
  void input_context::connect(uv_termpaint_integration * J_NOT_NULL integration) noexcept {
    integration->on_key.connect<&input_context::handle_key>(this);
    integration->on_char.connect<&input_context::handle_text>(this);
    integration->on_paste.connect<&input_context::handle_paste>(this);
  }

  void input_context::handle_text(strings::const_string_view c, input::modifier_mask) {
    if (accept_text_input) {
      on_text(c);
    }
  }

  void input_context::handle_paste(strings::const_string_view c) {
    if (accept_text_input) {
      on_paste(c);
    }
  }

  void input_context::handle_key(u32_t keysym, input::modifier_mask mods) {
    switch (keysym) {
    case XKB_KEY_Return:
      if (accept_text_input && enter_as_text && !mods) {
        on_text("\n");
      } else if (accept_text_input && shift_enter_as_text && mods == input::modifier::shift) {
        on_text("\n");
      } else if (enter_as_accept && !mods) {
        on_accept();
      } else if (ctrl_enter_as_accept && mods == input::modifier::ctrl) {
        on_accept();
      }
      break;
    case XKB_KEY_space:
      if (accept_text_input && space_as_text && !mods) {
        on_text(" ");
      }
      break;
    case XKB_KEY_Down:
      if (enable_movement && !mods) {
        on_move(movement::down);
      }
      break;
    case XKB_KEY_Up:
      if (enable_movement && !mods) {
        on_move(movement::up);
      }
      break;
    case XKB_KEY_Right:
      if (enable_movement) {
        on_move((mods && enable_word_movements) ? movement::word_right : movement::right);
      }
      break;
    case XKB_KEY_Left:
      if (enable_movement) {
        on_move((mods && enable_word_movements) ? movement::word_left : movement::left);
      }
      break;
    case XKB_KEY_Page_Up:
      if (enable_movement && enable_page_movements) {
        on_move(movement::page_up);
      }
      break;
    case XKB_KEY_Page_Down:
      if (enable_movement && enable_page_movements) {
        on_move(movement::page_down);
      }
      break;
    case XKB_KEY_Home:
      if (enable_movement && enable_line_begin_end && !mods) {
        on_move(movement::line_begin);
      } else if (enable_movement && ctrl_home_end_as_doc_begin_end && mods == input::modifier::ctrl) {
        on_move(movement::doc_begin);
      } else if (enable_movement && enable_visual_line_begin_end  && mods == input::modifier::shift) {
        on_move(movement::visual_line_begin);
      }
      break;
    case XKB_KEY_End:
      if (enable_movement && enable_line_begin_end && !mods) {
        on_move(movement::line_end);
      } else if (enable_movement && ctrl_home_end_as_doc_begin_end && mods == input::modifier::ctrl) {
        on_move(movement::doc_end);
      } else if (enable_movement && enable_visual_line_begin_end  && mods == input::modifier::shift) {
        on_move(movement::visual_line_end);
      }
      break;
    case XKB_KEY_Delete:
      if (enable_edit_actions) {
        on_edit((enable_delete_word && mods.has(input::modifier::ctrl))
                ? edit_action::delete_word_forward
                : edit_action::delete_forward);
      }
      break;
    case XKB_KEY_BackSpace:
      if (enable_edit_actions) {
        on_edit((enable_delete_word && mods.has(input::modifier::ctrl))
                ? edit_action::delete_word_backward
                : edit_action::delete_backward);
      }
      break;
    default:
      break;
    }
  }
}
