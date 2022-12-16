#pragma once

#include "tty/output_context.hpp"

namespace j::tty {
  struct output_context;
  struct input_context;

  struct component {
    size min_size;
    size cur_size;
    pos cur_pos;
    bool is_dirty:1 = true;
    bool is_layout_dirty:1 = true;
    bool is_dynamic_height:1 = false;
    bool has_dirty_children:1 = false;
    bool contains_focus:1 = false;
    bool accepts_focus:1 = false;
    component * parent = nullptr;

    virtual ~component();

    void set_dirty() noexcept;
    void set_layout_dirty() noexcept;

    virtual void render(output_context & ctx) noexcept = 0;
    virtual void relayout(output_context & ctx) noexcept = 0;

    virtual void resize(size sz) noexcept;
    virtual void reposition(pos pos) noexcept;

    virtual input_context * get_input_context() noexcept;

    void set_contains_focus(bool value) noexcept;
  };
}
