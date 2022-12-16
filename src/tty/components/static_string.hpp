#pragma once

#include "tty/components/line_table.hpp"
#include "tty/attr.hpp"
#include "tty/component.hpp"

namespace j::tty::inline components {
  struct static_string final : component {
    explicit static_string(strings::const_string_view str = "") noexcept;

    void set_text(strings::const_string_view str);

    void render(output_context & ctx) noexcept override;
    void relayout(output_context & ctx) noexcept override;

    line_table table;

    J_ALWAYS_INLINE void set_text_style(const strings::style & style) noexcept {
      table.set_text_style(style);
    }
    J_ALWAYS_INLINE void set_initial_prefix_style(const strings::style & style) noexcept {
      table.set_initial_prefix_style(style);
    }
    J_ALWAYS_INLINE void set_prefix_style(const strings::style & style) noexcept {
      table.set_prefix_style(style);
    }
  };
}
