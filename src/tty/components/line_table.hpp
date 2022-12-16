#pragma once

#include "strings/unicode/rope/rope_utf8.hpp"
#include "containers/vector.hpp"
#include "strings/styling/style.hpp"
#include "tty/output_context.hpp"

namespace j::tty::inline components {
  struct line_table final {
    struct line final {
      i16_t width;
      i16_t bytes;
    };
    struct hard_line final {
      i16_t width = -1;
      strings::marker end;
      vector<line> lines;
    };

    explicit line_table(i16_t prefix_width = 2) noexcept;

    void compute_hard_lines(output_context & ctx);
    void compute_all_soft_lines(output_context & ctx, i16_t width);

    strings::rope_utf8 value;
    i16_t total_lines = -1;
    i16_t max_line_width = -1;
    i16_t prefix_width = 2;

    vector<hard_line> hard_lines;

    strings::rope_utf8_byte_iterator line_begin(strings::rope_utf8_byte_iterator it) noexcept;
    strings::rope_utf8_byte_iterator line_end(strings::rope_utf8_byte_iterator it) noexcept;
    strings::rope_utf8_byte_iterator visual_line_begin(strings::rope_utf8_byte_iterator it) noexcept;
    strings::rope_utf8_byte_iterator visual_line_end(strings::rope_utf8_byte_iterator it) noexcept;

    attr text_style;
    attr initial_prefix_style;
    attr prefix_style;

    void set_text(strings::const_string_view str);
    void set_text_style(const strings::style & style) noexcept;
    void set_initial_prefix_style(const strings::style & style) noexcept;
    void set_prefix_style(const strings::style & style) noexcept;
    void render(output_context & ctx);

    strings::string prefix = "+ ";
    strings::string initial_prefix = "> ";

    pos get_pos(output_context & ctx, strings::const_rope_utf8_byte_iterator it) const noexcept;
    strings::rope_utf8_byte_iterator cursor_at(output_context & ctx, pos p) noexcept;
    cursor_offsets offsets(output_context & ctx, pos p);
  };
}
