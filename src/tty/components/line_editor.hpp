#pragma once

#include "tty/attr.hpp"
#include "tty/input_context.hpp"
#include "tty/components/line_table.hpp"
#include "tty/component.hpp"
#include "signals/signal.hpp"
#include "strings/styling/style.hpp"

namespace j::tty::inline components {
  struct line_editor final : component {
    explicit line_editor(uv_termpaint_integration * J_NOT_NULL ig) noexcept;

    void set_text(strings::const_string_view str);
    void set_cursor_position(u32_t index);

    void on_text(strings::const_string_view str);
    void on_move(movement move);
    void on_edit(edit_action action);

    input_context in_ctx;

    strings::rope_utf8_byte_iterator cursor;

    i32_t cursor_prev_offset = -1;
    i32_t cursor_next_offset = -1;
    i32_t cursor_up_offset = -1;
    i32_t cursor_down_offset = -1;

    line_table table;

    signals::signal<void (strings::string &&)> on_accept;
    void handle_accept();

    void render(output_context & ctx) noexcept override;
    void relayout(output_context & ctx) noexcept override;

    J_ALWAYS_INLINE void set_text_style(const strings::style & style) noexcept {
      table.set_text_style(style);
    }
    J_ALWAYS_INLINE void set_prompt_style(const strings::style & style) noexcept {
      table.set_initial_prefix_style(style);
    }
    J_ALWAYS_INLINE void set_continuation_style(const strings::style & style) noexcept {
      table.set_prefix_style(style);
    }

    void update_cursor_up_down_offsets(output_context & ctx, pos cur);
  };
}
