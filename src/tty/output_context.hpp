#pragma once

#include "tty/common.hpp"
#include "signals/signal.hpp"
#include "strings/string_view.hpp"
#include "strings/unicode/rope/rope_utf8_view.hpp"
#include "tty/attr.hpp"

extern "C" {
  struct termpaint_text_measurement_;
}

namespace j::tty {
  struct string_metrics final {
    i16_t bytes = -1;
    i16_t columns = -1;
  };

  struct cursor_offsets final {
    i16_t before = -1;
    i16_t after = -1;
  };

  struct output_context final {
    J_ALWAYS_INLINE output_context(
      uv_termpaint_integration * J_NOT_NULL integration,
      struct component * J_NOT_NULL c,
      pos scroll = {0, 0},
      size viewport_size = {-1, -1}
    ) noexcept
      : c(c),
        integration(integration),
        scroll(scroll),
        viewport_size(viewport_size)
    { }

    void clear();
    void clear(const attr & a);

    void fill(int codepoint, const attr & a);

    string_metrics measure(strings::const_string_view sv, string_metrics limit = {}, bool pend = false) const;
    string_metrics measure(const strings::const_rope_utf8_view & v, string_metrics limit = {}, bool pend = false) const;

    cursor_offsets offsets(i16_t col, strings::const_string_view sv, string_metrics limit = {});
    cursor_offsets offsets(i16_t col, const strings::const_rope_utf8_view & v, string_metrics limit = {});

    void write(i16_t x, i16_t y, strings::const_string_view sv);
    void write(i16_t x, i16_t y, const strings::const_rope_utf8_view & v);
    void write(i16_t x, i16_t y, strings::const_string_view sv, const attr & a);
    void write(i16_t x, i16_t y, const strings::const_rope_utf8_view & v, const attr & a);

    void flush();

    /// Set cursor style. or whether it is hidden, or blinking.
    void set_cursor_style(cursor_style style, bool blink);
    void set_cursor_pos(pos cursor_pos);

    i16_t width() const noexcept;
    i16_t height() const noexcept;

    i16_t x(i16_t local_x = 0) const noexcept;
    i16_t y(i16_t local_y = 0) const noexcept;

    i16_t min_x() const noexcept;
    i16_t min_y() const noexcept;
    i16_t max_x() const noexcept;
    i16_t max_y() const noexcept;

    bool is_x_visible(i16_t x) const noexcept {
      return x >= min_x() && x < max_x();
    }

    bool is_y_visible(i16_t y) const noexcept {
      return y >= min_y() && y < max_y();
    }

    bool is_visible(i16_t x, i16_t y) const noexcept {
      return is_y_visible(y) && is_x_visible(x);
    }

    bool is_visible(pos p) const noexcept {
      return is_y_visible(p.y) && is_x_visible(p.x);
    }

    output_context enter(component * J_NOT_NULL c, pos scroll = {0, 0}, size viewport_size = {-1, -1}) noexcept;

    struct component * c = nullptr;
    struct uv_termpaint_integration * integration = nullptr;
    pos scroll{0, 0};
    size viewport_size{-1, -1};

  private:
    void check_status() noexcept;
  };
}
