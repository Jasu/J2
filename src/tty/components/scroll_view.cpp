#include "tty/components/scroll_view.hpp"
#include "logging/global.hpp"

namespace j::tty::inline components {

  scroll_view::scroll_view(axis scroll_axis, scroll_mode mode, component * child) noexcept
    : scroll_axis(scroll_axis),
      mode(mode)
  {
    set_child(child);
  }

  void scroll_view::set_child(component * c) noexcept {
    if (child) {
      child->parent = nullptr;
    }
    child = c;
    if (child) {
      child->parent = this;
      accepts_focus = child->accepts_focus;
      set_contains_focus(child->contains_focus);
    }
    set_layout_dirty();
  }

  void scroll_view::render(output_context & ctx) noexcept {
    if (is_dirty || child->is_dirty || child->has_dirty_children) {
      if (is_dirty) {
        ctx.clear();
      }
      auto child_ctx = ctx.enter(child, scroll, cur_size);
      child->render(child_ctx);
      child->is_dirty = false;
      child->has_dirty_children = false;
    }
  }

  void scroll_view::relayout(output_context & ctx) noexcept {
    child->reposition(cur_pos);
    if (child->is_dynamic_height) {
      child->resize({ cur_size.width, -1 });
    } else {
      child->resize(cur_size.max(child->min_size));
    }
    auto child_ctx = ctx.enter(child, scroll, cur_size);
    child->relayout(child_ctx);
    J_ASSUME(child->cur_size.height >= 0 && child->cur_size.width >= 0);
    child->is_layout_dirty = false;

    switch (mode) {
    case scroll_mode::bottom:
      scroll = {0, (i16_t)(cur_size.height - child->cur_size.height)};
      break;
    case scroll_mode::top:
      scroll = {0, 0};
      break;
    case scroll_mode::manual:
      break;
    }
  }
}
