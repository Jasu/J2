#include "tty/components/linear_layout.hpp"
#include "logging/global.hpp"

namespace j::tty::inline components {
  linear_layout::linear_layout(axis a) noexcept
    : primary_axis(a)
  { }

  void linear_layout::remove_child(component * J_NOT_NULL c) noexcept {
    for (auto it = children.begin(), end = children.end(); it != end; ++it) {
      if (*it == c) {
        children.erase(it);
        return;
      }
    }
    J_FAIL("Child not found.");
  }

  void linear_layout::append_child(component * J_NOT_NULL c) noexcept {
    children.emplace_back(c);
    add_child(c);
  }

  void linear_layout::prepend_child(component * J_NOT_NULL c) noexcept {
    children.emplace_front(c);
    add_child(c);
  }

  void linear_layout::add_child(component * J_NOT_NULL c) noexcept {
    c->parent = this;
    c->set_layout_dirty();
    accepts_focus |= c->accepts_focus;
    is_dynamic_height |= c->is_dynamic_height;
    set_contains_focus(contains_focus | c->contains_focus);
  }

  void linear_layout::relayout(output_context & ctx) noexcept {
    J_ASSUME(cur_size.width > 0);
    i16_t sum_primary = 0;

    for (auto ch : children) {
      if (is_layout_dirty || ch->is_layout_dirty) {
        if (ch->is_dynamic_height && primary_axis == axis::vertical) {
          ch->resize({cur_size.width, -1});
        } else {
          ch->resize({cur_size.width, ch->cur_size.height});
        }
        auto ch_ctx = ctx.enter(ch);
        ch->relayout(ch_ctx);
        ch->is_layout_dirty = false;
      }
      i16_t sz = ch->cur_size.get(primary_axis);
      J_ASSUME(sz >= 0);
      sum_primary += sz;
    }

    pos child_pos = cur_pos;
    i16_t size_left = cur_size.get(primary_axis);
    if (size_left >= 0) {
      size_left -= sum_primary;
    } else {
      size_left = 0;
    }
    i16_t sz_per_child = size_left / children.size();
    i16_t sz_err = size_left - sz_per_child * children.size();

    for (auto ch : children) {
      ch->reposition(child_pos);
      size ch_size = ch->cur_size;
      // ch_size.set(secondary_axis, max_secondary);
      ch_size.add(primary_axis, sz_per_child);
      if (sz_err) {
        --sz_err;
        ch_size.add(primary_axis, 1);
      }
      ch->cur_size = ch_size;
      child_pos.add(primary_axis, ch_size.get(primary_axis));
    }

    if (is_dynamic_height && primary_axis == axis::vertical) {
      cur_size.height = sum_primary;
    }
  }

  void linear_layout::render(output_context & ctx) noexcept {
    if (is_dirty || has_dirty_children) {
      if (is_dirty) {
        ctx.clear();
      }
      for (auto ch : children) {
        if (is_dirty || ch->is_dirty || ch->has_dirty_children) {
          auto ch_ctx = ctx.enter(ch);
          ch->render(ch_ctx);
          ch->is_dirty = false;
          ch->has_dirty_children = false;
        }
      }
    }
  }
}
