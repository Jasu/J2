#include "divider.hpp"
#include "logging/global.hpp"
#include "strings/unicode/utf8.hpp"
#include "tty/termpaint_style_map.hpp"

namespace j::tty::inline components {

  divider::divider() noexcept
    : codepoint((i32_t)strings::utf8_get_code_point("─")),
      divider_style(style_map->get_attr(strings::styling::g_empty_style))
  {
  }

  void divider::set_character(u32_t codepoint) noexcept {
    this->codepoint = (i32_t)codepoint;
    set_dirty();
  }
  void divider::set_character(strings::const_string_view str) noexcept {
    J_ASSERT(str);
    codepoint = (i32_t)strings::utf8_get_code_point(str.data());
    set_dirty();
  }

  void divider::set_style(const strings::style & style) noexcept {
    divider_style = style_map->get_attr(style);
    set_dirty();
  }

  void divider::render(output_context & ctx) noexcept {
    ctx.fill(codepoint, divider_style);
  }

  void divider::relayout(output_context &) noexcept {
    cur_size.height = 1U;
  }
}
