#pragma once

#include "input/modifier.hpp"
#include "signals/signal.hpp"
#include "strings/string_view.hpp"

namespace j::tty {
  struct uv_termpaint_integration;

  enum class movement : u8_t {
    up,
    down,
    left,
    right,

    page_up,
    page_down,

    word_left,
    word_right,

    line_begin,
    line_end,

    visual_line_begin,
    visual_line_end,

    doc_begin,
    doc_end,
  };

  enum class edit_action : u8_t {
    delete_backward,
    delete_forward,

    delete_word_backward,
    delete_word_forward,
  };

  struct input_context final {
    bool accept_text_input:1 = false;
    bool space_as_text:1 = false;
    bool enter_as_text:1 = false;
    bool shift_enter_as_text:1 = false;

    bool enter_as_accept:1 = false;
    bool ctrl_enter_as_accept:1 = false;

    bool enable_movement:1 = false;
    bool enable_word_movements:1 = false;
    bool enable_page_movements:1 = false;
    bool enable_line_begin_end:1 = false;
    bool enable_visual_line_begin_end:1 = false;
    bool ctrl_home_end_as_doc_begin_end:1 = false;
    bool shift_home_end_as_doc_begin_end:1 = false;

    bool enable_edit_actions:1 = false;
    bool enable_delete_word:1 = false;

    signals::signal<void (strings::const_string_view)> on_text;
    signals::signal<void (strings::const_string_view)> on_paste;
    signals::signal<void (movement)> on_move;
    signals::signal<void (edit_action)> on_edit;
    signals::signal<void ()> on_accept;

    void connect(uv_termpaint_integration * J_NOT_NULL integration) noexcept;

  private:
    void handle_text(strings::const_string_view, input::modifier_mask);
    void handle_paste(strings::const_string_view);
    void handle_key(u32_t keysym, input::modifier_mask);
  };
}
