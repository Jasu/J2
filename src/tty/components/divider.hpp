#pragma once

#include "tty/attr.hpp"
#include "tty/component.hpp"
#include "strings/styling/style.hpp"

namespace j::tty::inline components {
  struct divider final : component {
    divider() noexcept;

    void set_character(u32_t codepoint) noexcept;
    void set_character(strings::const_string_view str) noexcept;
    void set_style(const strings::style & style) noexcept;

    void render(output_context & ctx) noexcept override;
    void relayout(output_context & ctx) noexcept override;

    int codepoint;
    attr divider_style;
  };
}
