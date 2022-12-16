#include "tty/output_context.hpp"
#include "logging/global.hpp"
#include "tty/component.hpp"
#include "tty/uv_termpaint_integration.hpp"
#include "strings/unicode/rope/rope_debug.hpp"

namespace j::tty {
  namespace {
    const int cursor_styles[]{
      -1,
      TERMPAINT_CURSOR_STYLE_TERM_DEFAULT,
      TERMPAINT_CURSOR_STYLE_BLOCK,
      TERMPAINT_CURSOR_STYLE_UNDERLINE,
      TERMPAINT_CURSOR_STYLE_BAR,
    };

    void init_measurement(termpaint_text_measurement * J_NOT_NULL measurement, string_metrics limit) noexcept {
      J_ASSERT(limit.bytes && limit.columns);
      termpaint_text_measurement_reset(measurement);
      if (limit.columns >= 0) {
        termpaint_text_measurement_set_limit_width(measurement, limit.columns);
      }
      if (limit.bytes >= 0) {
        termpaint_text_measurement_set_limit_ref(measurement, limit.bytes);
      }
    }

    template<typename S>
    cursor_offsets offsets_impl(output_context * J_NOT_NULL c, i16_t col, const S & sv, string_metrics limit) noexcept {
      J_ASSUME(col >= 0);
      auto prev = col > 1
        ? c->measure(sv, { .bytes = limit.bytes, .columns = (limit.columns >= 0) ? min<i16_t>(col - 1, limit.columns) : (i16_t)((col - 1))}).bytes
        : 0;
      auto cur = col > 0
        ? c->measure(sv, { .bytes = limit.bytes, .columns = (limit.columns >= 0) ? min(col, limit.columns) : col}).bytes
        : 0;
      auto next = c->measure(sv, { .bytes = limit.bytes, .columns = (limit.columns >= 0) ? min<i16_t>(col + 1, limit.columns) : (i16_t)(col + 1)}).bytes;
      return {
        .before = (i16_t)(cur - prev),
        .after = (i16_t)(next - cur),
      };
    }
  }

  namespace s = strings;

  void output_context::flush() {
    termpaint_terminal_flush(integration->term, false);
  }

  void output_context::set_cursor_style(cursor_style style, bool blink) {
    termpaint_terminal_set_cursor_visible(integration->term, style != cursor_style::hidden);
    int termstyle = cursor_styles[(u8_t)style];
    if (termstyle >= 0) {
      termpaint_terminal_set_cursor_style(integration->term, termstyle, blink);
    }
  }

  void output_context::set_cursor_pos(pos cursor_pos) {
    if (is_visible(cursor_pos)) {
      termpaint_terminal_set_cursor_position(integration->term, this->x(cursor_pos.x), this->y(cursor_pos.y));
    }
  }

  string_metrics output_context::measure(strings::const_string_view sv, string_metrics limit, bool pend) const {
    if (!limit.columns || !limit.bytes || sv.empty()) {
      return { 0, 0 };
    }
    init_measurement(integration->measurement, limit);
    termpaint_text_measurement_feed_utf8(integration->measurement, sv.begin(), sv.size(), true);
    return {
      .bytes = pend
        ? (i16_t)termpaint_text_measurement_pending_ref(integration->measurement)
        : (i16_t)termpaint_text_measurement_last_ref(integration->measurement),
      .columns = (i16_t)termpaint_text_measurement_last_width(integration->measurement),
    };
  }

  string_metrics output_context::measure(const strings::const_rope_utf8_view & v, string_metrics limit, bool pend) const {
    if (!limit.columns || !limit.bytes || v.empty()) {
      return { 0, 0 };
    }
    init_measurement(integration->measurement, limit);
    v.for_each_chunk([&](auto sv) {
      termpaint_text_measurement_feed_utf8(integration->measurement, sv.begin(), sv.size(), false);
    });
    termpaint_text_measurement_feed_utf8(integration->measurement, nullptr, 0, true);
    return {
      .bytes = pend
        ? (i16_t)termpaint_text_measurement_pending_ref(integration->measurement)
        : (i16_t)termpaint_text_measurement_last_ref(integration->measurement),
      .columns = (i16_t)termpaint_text_measurement_last_width(integration->measurement),
    };
  }

  cursor_offsets output_context::offsets(i16_t col, strings::const_string_view sv, string_metrics limit) {
    return offsets_impl(this, col, sv, limit);
  }

  cursor_offsets output_context::offsets(i16_t col, const strings::const_rope_utf8_view & v, string_metrics limit) {
    return offsets_impl(this, col, v, limit);
  }

  void output_context::write(i16_t x, i16_t y, strings::const_string_view sv) {
    if (is_visible(x, y)) {
      termpaint_surface_write_with_len_colors(integration->surf, this->x(x), this->y(y), sv.begin(), sv.size(),
                                              TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);
    }
  }

  void output_context::write(i16_t x, i16_t y, const strings::const_rope_utf8_view & v) {
    if (is_visible(x, y)) {
      v.for_each_chunk([&](auto sv) {
        write(x, y, sv);
        x += measure(sv).columns;
      });
    }
  }

  void output_context::write(i16_t x, i16_t y, strings::const_string_view sv, const attr & a) {
    if (is_visible(x, y)) {
      termpaint_surface_write_with_len_attr(integration->surf, this->x(x), this->y(y), sv.begin(), sv.size(), a.get());
    }
  }

  void output_context::write(i16_t x, i16_t y, const strings::const_rope_utf8_view & v, const attr & a) {
    if (is_visible(x, y)) {
      v.for_each_chunk([&](auto sv) {
        write(x, y, sv, a);
        x += measure(sv).columns;
      });
    }
  }

  void output_context::clear() {
    termpaint_surface_clear_rect(integration->surf, x(), y(), max_x() - min_x(), max_y() - min_y(),
                                 TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);
  }

  void output_context::clear(const attr & a) {
    J_ASSERT(a && c && c->cur_pos.x >= 0 && c->cur_pos.y >= 0 && c->cur_size.width > 0 && c->cur_size.height > 0);
    termpaint_surface_clear_rect_with_attr(integration->surf, c->cur_pos.x, c->cur_pos.y, c->cur_size.width, c->cur_size.height, a.get());
  }
  void output_context::fill(int codepoint, const attr & a) {
    termpaint_surface_clear_rect_with_attr_char(integration->surf, x(), y(), max_x() - min_x(), max_y() - min_y(),
                                                a.get(), codepoint);
  }

  i16_t output_context::width() const noexcept {
    J_ASSUME(c && c->cur_size.width > 0);
    return c->cur_size.width;
  }
  i16_t output_context::height() const noexcept {
    J_ASSUME(c && c->cur_size.height > 0);
    return c->cur_size.height;
  }

  i16_t output_context::x(i16_t local_x) const noexcept {
    J_ASSUME(local_x >= 0);
    J_ASSUME(c && c->cur_pos.x >= 0);
    return c->cur_pos.x + local_x + scroll.x;
  }

  i16_t output_context::y(i16_t local_y) const noexcept {
    J_ASSUME(local_y >= 0);
    J_ASSUME(c && c->cur_pos.y >= 0);
    return c->cur_pos.y + local_y + scroll.y;
  }
  i16_t output_context::min_x() const noexcept {
    return max(0, -scroll.x);
  }
  i16_t output_context::min_y() const noexcept {
    return max(0, -scroll.y);
  }
  i16_t output_context::max_x() const noexcept {
    if (c->cur_size.width >= 0 && viewport_size.width >= 0) {
      return min(width(), viewport_size.width - scroll.x);
    } else if (c->cur_size.width >= 0) {
      return width();
    } else {
      return viewport_size.width - scroll.x;
    }
  }

  i16_t output_context::max_y() const noexcept {
    if (c->cur_size.height >= 0 && viewport_size.height >= 0) {
      return min(height(), viewport_size.height - scroll.y);
    } else if (c->cur_size.height >= 0) {
      return height();
    } else {
      return viewport_size.height - scroll.y;
    }
  }

  output_context output_context::enter(component * J_NOT_NULL c, pos new_scroll, size viewport_size) noexcept {
    return { integration, c, scroll + new_scroll, viewport_size };
  }
}
