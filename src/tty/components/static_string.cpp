#include "tty/components/static_string.hpp"

namespace j::tty::inline components {
  namespace s = strings;

  static_string::static_string(strings::const_string_view str) noexcept
    : table(2)
  {
    min_size = { 3, 1 };
    is_dynamic_height = true;
    table.value = str;
  }

  void static_string::set_text(strings::const_string_view str) {
    table.set_text(str);
    set_layout_dirty();
  }

  void static_string::render(output_context & ctx) noexcept {
    table.render(ctx);
  }

  void static_string::relayout(output_context & ctx) noexcept {
    table.compute_hard_lines(ctx);
    table.compute_all_soft_lines(ctx, cur_size.width);
    cur_size.height = table.total_lines;
  }
}
