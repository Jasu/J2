#pragma once

#include "tty/component.hpp"
#include "containers/vector.hpp"

namespace j::tty::inline components {
  struct linear_layout final : component {
    explicit linear_layout(axis a) noexcept;

    void append_child(component * J_NOT_NULL c) noexcept;
    void prepend_child(component * J_NOT_NULL c) noexcept;
    void remove_child(component * J_NOT_NULL c) noexcept;

    void render(output_context & ctx) noexcept override;
    void relayout(output_context & ctx) noexcept override;

    noncopyable_vector<component *> children;
    axis primary_axis;
    i16_t scroll = 0;
  private:
    void add_child(component * J_NOT_NULL c) noexcept;
  };
}
