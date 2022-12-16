#include "tty/component.hpp"

namespace j::tty {
  component::~component() {
  }

  void component::set_dirty() noexcept {
    if (!is_dirty) {
      is_dirty = true;
      for (component * p = parent; p && !p->has_dirty_children; p = p->parent) {
        p->has_dirty_children = true;
      }
    }
  }

  void component::set_layout_dirty() noexcept {
    if (!is_layout_dirty) {
      is_dirty = true;
      is_layout_dirty = true;
      for (component * p = parent; p && !p->is_layout_dirty; p = p->parent) {
        p->is_dirty = true;
        p->is_layout_dirty = true;
        p->has_dirty_children = true;
      }
    }
  }

  void component::resize(size sz) noexcept {
    if (cur_size != sz) {
      cur_size = sz;
      set_layout_dirty();
    }
  }

  void component::reposition(pos pos) noexcept {
    if (cur_pos != pos) {
      cur_pos = pos;
      set_dirty();
    }
  }

  input_context * component::get_input_context() noexcept {
    return nullptr;
  }

  void component::set_contains_focus(bool value) noexcept {
    if (contains_focus != value) {
      contains_focus = value;
      if (parent) {
        parent->set_contains_focus(value);
      }
    }
  }
}
