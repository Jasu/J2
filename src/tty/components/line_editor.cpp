#include "tty/components/line_editor.hpp"
#include "logging/global.hpp"
#include "strings/unicode/rope/rope_debug.hpp"
#include "containers/trees/rope_tree_debug.hpp"

namespace j::tty::inline components {
  line_editor::line_editor(uv_termpaint_integration * J_NOT_NULL ig) noexcept
    : in_ctx{
        .accept_text_input = true,
        .space_as_text = true,
        .enter_as_text = true,
        .shift_enter_as_text = true,

        .ctrl_enter_as_accept = true,

        .enable_movement = true,
        .enable_word_movements = true,
        .enable_line_begin_end = true,
        .enable_visual_line_begin_end = true,
        .ctrl_home_end_as_doc_begin_end = true,

        .enable_edit_actions = true,
        .enable_delete_word = true,
      },
      table(2)
  {
    min_size = { 3, 1 };
    is_dynamic_height = true;
    accepts_focus = true;
    in_ctx.on_text.connect<&line_editor::on_text>(this);
    in_ctx.on_paste.connect<&line_editor::on_text>(this);
    in_ctx.on_edit.connect<&line_editor::on_edit>(this);
    in_ctx.on_move.connect<&line_editor::on_move>(this);
    in_ctx.on_accept.connect<&line_editor::handle_accept>(this);
    in_ctx.connect(ig);
    cursor = table.value.bytes().begin();
  }

  void line_editor::render(output_context & ctx) noexcept {
    table.render(ctx);

    auto cur = table.get_pos(ctx, cursor);
    auto limits = table.offsets(ctx, cur);
    cursor_prev_offset = max(1, limits.before);
    cursor_next_offset = max(1, limits.after);

    ctx.set_cursor_pos(cur);
    ctx.set_cursor_style(cursor_style::bar, true);
    update_cursor_up_down_offsets(ctx, cur);
  }

  void line_editor::relayout(output_context & ctx) noexcept {
    table.compute_hard_lines(ctx);
    table.compute_all_soft_lines(ctx, cur_size.width);
    cur_size.height = table.total_lines;
  }

  void line_editor::set_cursor_position(u32_t index) {
    cursor = table.value.bytes().iterate_at(index);
    cursor_up_offset = cursor_down_offset = -1;
    set_dirty();
  }

  void line_editor::set_text(strings::const_string_view str) {
    table.set_text(str);
    cursor = table.value.bytes().begin();
    set_layout_dirty();
    cursor_up_offset = cursor_down_offset = -1;
  }

  void line_editor::on_text(strings::const_string_view str) {
    auto pos = cursor.position() + str.size();

    cursor = table.value.insert(cursor, str);
    // J_DUMP_ROPE_TREE(table.value.m_tree);
    strings::unicode::rope::validate_iterator(cursor);
    if (pos != cursor.position()) {
      cursor = table.value.bytes().iterate_at(pos);
      strings::unicode::rope::validate_iterator(cursor);
    }
    set_layout_dirty();
    cursor_up_offset = cursor_down_offset = -1;
  }

  void line_editor::on_move(movement move) {
    if (table.value.empty()) {
      return;
    }
    strings::rope_utf8_byte_iterator min = table.value.bytes().begin(),
      max = table.value.bytes().end();

    switch (move) {
    case movement::left:
      if (cursor != min) {
        cursor -= cursor_prev_offset;
        break;
      }
      return;
    case movement::right:
      if (cursor != max) {
        cursor += cursor_next_offset;
        break;
      }
      return;
    case movement::line_begin:
      cursor = table.line_begin(cursor);
      break;
    case movement::visual_line_begin:
      cursor = table.visual_line_begin(cursor);
      break;
    case movement::visual_line_end:
      cursor = table.visual_line_end(cursor);
      break;
    case movement::doc_begin:
      cursor = min;
      break;
    case movement::doc_end:
      cursor = max;
      break;
    case movement::line_end:
      cursor = table.line_end(cursor);
      break;

    case movement::word_left:
    case movement::word_right:
      return;
    case movement::page_up:
    case movement::page_down:
      return;
    case movement::up:
      if (cursor_up_offset > 0) {
        cursor -= cursor_up_offset;
        break;
      }
      return;
    case movement::down:
      if (cursor_down_offset > 0) {
        cursor += cursor_down_offset;
        break;
      }
      return;
    }

    cursor_up_offset = cursor_down_offset = -1;
    strings::unicode::rope::validate_iterator(cursor);
    set_dirty();
  }

  void line_editor::on_edit(edit_action action) {
    if (table.value.empty()) {
      return;
    }
    strings::rope_utf8_byte_iterator min = table.value.bytes().begin(), max = table.value.bytes().end();
    switch (action) {
    case edit_action::delete_backward:
      if (cursor != min) {
        u32_t count = strings::utf8_code_point_bytes_backwards(cursor.m_char - 1);
        cursor -= count;
        cursor = table.value.erase(cursor, count);
        break;
      }
      return;
    case edit_action::delete_forward:
      if (cursor != max) {
        u32_t count = strings::utf8_code_point_bytes(cursor.m_char);
        cursor = table.value.erase(cursor, count);
        break;
      }
      return;
    case edit_action::delete_word_backward:
    case edit_action::delete_word_forward:
      return;
    }
    cursor_up_offset = cursor_down_offset = -1;
    strings::unicode::rope::validate_iterator(cursor);
    set_layout_dirty();
  }

  void line_editor::handle_accept() {
    auto view = table.value.view_at(table.value.bytes().begin(), table.value.bytes().end());
    on_accept((strings::string)view);
  }

  void line_editor::update_cursor_up_down_offsets(output_context & ctx, pos cur) {
    cursor_up_offset = cursor_down_offset = 0;
    auto cur_offset = cursor.position();
    if (cur.y > 0) {
      cursor_up_offset = cur_offset - table.cursor_at(ctx, { cur.x, (i16_t)(cur.y - 1) }).position();
    }
    if (cur.y < table.total_lines - 1) {
      cursor_down_offset = table.cursor_at(ctx, { cur.x, (i16_t)(cur.y + 1) }).position() - cur_offset;
    }
  }
}
