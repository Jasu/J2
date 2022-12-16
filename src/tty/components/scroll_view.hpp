#pragma once

#include "tty/component.hpp"

namespace j::tty::inline components {
  enum scroll_mode : u8_t {
    manual,
    bottom,
    top,
  };

  struct scroll_view final : component {
    explicit scroll_view(axis scroll_axis, scroll_mode mode = scroll_mode::manual, component * child = nullptr) noexcept;

    void set_child(component * c) noexcept;

    void render(output_context & ctx) noexcept override;
    void relayout(output_context & ctx) noexcept override;

    pos scroll{0, 0};
    axis scroll_axis;
    component * child = nullptr;
    scroll_mode mode;
  };
}
