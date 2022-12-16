#include "code_writer.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatting/pad.hpp"

namespace j::meta {
  namespace s = strings;
  [[nodiscard]] bool code_writer_settings::operator<(const code_writer_settings & rhs) const {
    if (!begin_token != !rhs.begin_token) {
      return !begin_token;
    } else if (begin_token) {
      if (int diff = ::j::strcmp(begin_token, rhs.begin_token)) {
        return diff < 0;
      }
    }
    if (pack_empty_lines.value() != rhs.pack_empty_lines.value()) {
      return pack_empty_lines.value() < rhs.pack_empty_lines.value();
    }
    if (indent != rhs.indent) {
      return indent < rhs.indent;
    }
    return inner_indent < rhs.inner_indent;
  }

  code_writer::code_writer(strings::formatted_sink * J_NOT_NULL to, const code_writer_settings settings) noexcept
      : to(to),
        indent(settings.indent >= 0 ? settings.indent : 0U),
        inner_indent(settings.inner_indent >= 0 ? settings.inner_indent : 0U),
        pack_empty_lines(!settings.pack_empty_lines.is_false()),
        begin_token(settings.begin_token ? settings.begin_token : "")
  { }

  void code_writer::configure(const code_writer_settings & settings) noexcept {
    J_ASSERT(indent >= 0 && inner_indent >= 0);
    if (settings.indent >= 0) {
      indent = settings.indent;
    }
    if (settings.inner_indent >= 0) {
      inner_indent = settings.inner_indent;
    }
    if (settings.begin_token) {
      begin_token = settings.begin_token;
    }
    pack_empty_lines = settings.pack_empty_lines / pack_empty_lines;
  }

  code_writer_guard code_writer::enter(const code_writer_settings & settings) noexcept {
    J_ASSERT(indent >= 0 && inner_indent >= 0);
    code_writer_guard result{
      this,
      {
        begin_token,
        pack_empty_lines,
        (i8_t)indent,
        (i8_t)inner_indent,
      }
    };
    configure(settings);
    return result;
  }

  [[nodiscard]] code_writer_guard code_writer::begin_indent() noexcept {
    state = indented;
    return enter({ .indent = (i8_t)cur_col });
  }

  [[nodiscard]] code_writer_guard code_writer::begin_indent_inner() noexcept {
    return enter({ .inner_indent = (i8_t)(cur_col - indent - j::strlen(begin_token)) });
  }

  [[nodiscard]] code_writer_guard code_writer::descend_guarded(i8_t indent) noexcept {
    return enter({ .indent = (i8_t)(this->indent + (indent >= 0 ? indent : indent_step)) });
  }

  [[nodiscard]] code_writer_guard code_writer::descend_inner(i8_t indent) noexcept {
    return enter({ .inner_indent = (i8_t)(this->inner_indent + (indent >= 0 ? indent : indent_step)) });
  }

  void code_writer::write_empty_line() noexcept {
    if (state == token || state == mid_line) {
      to->write_unformatted("\n");
      state = line_begin;
    }
    if (*begin_token && state == line_begin) {
      s::write_spaces(*to->sink, indent);
      state = indented;
    }
    if (*begin_token && state == indented) {
      to->write_unformatted(begin_token);
    }
    to->write_unformatted("\n");
    is_between_sections = empty_line_queued = break_queued = false;
    state = line_begin;
    cur_col = 0U;
  }

  bool code_writer::line_break() noexcept {
    if (pack_empty_lines) {
      break_queued = state == mid_line;
      return false;
    } else {
      write_empty_line();
      return true;
    }
  }

  void code_writer::begin_line() noexcept {
    begin_line_no_token();
    if (state == indented) {
      if (*begin_token) {
        cur_col += to->write_unformatted(begin_token);
      }
      state = token;
    }
    if (state == token) {
      s::write_spaces(*to->sink, inner_indent);
      cur_col += inner_indent;
      state = mid_line;
    }
  }

  void code_writer::begin_line_no_token() noexcept {
    begin_line_no_indent();
    if (state == line_begin) {
      s::write_spaces(*to->sink, indent);
      cur_col = indent;
      state = indented;
    }
    break_queued = false;
  }

  void code_writer::begin_line_no_indent() noexcept {
    if (is_between_sections || empty_line_queued) {
      write_empty_line();
    } else if (state == mid_line || state == token) {
      to->write_unformatted("\n");
      cur_col = 0U;
      state = line_begin;
    }
    break_queued = false;
  }

  bool code_writer::begin_write() noexcept {
    if (is_between_sections || empty_line_queued || break_queued || state <= token) {
      begin_line();
      return true;
    }
    return false;
  }

  bool code_writer::write(strings::const_string_view str, strings::style st) noexcept {
    if (!str) {
      return false;
    }
    begin_write();
    to->write_unformatted(st, str);
    cur_col += str.size();
    state = mid_line;
    return true;
  }

  bool code_writer::newline() noexcept {
    if (state <= token && pack_empty_lines) {
      break_queued = true;
      return false;
    }
    to->write_unformatted("\n");
    cur_col = 0U;
    state = line_begin;
    return true;
  }

  bool code_writer::write_line(s::const_string_view str, strings::style st) noexcept {
    if (!str || str == "\n") {
      if (!pack_empty_lines || !is_between_sections) {
        return line_break();
      }
      return false;
    }
    begin_line();
    to->write_unformatted(st, str);
    if (str.back() != '\n') {
      to->write_unformatted("\n");
    }
    state = line_begin;
    cur_col = 0U;
    return true;
  }

  bool code_writer::write_with_line_breaks(strings::const_string_view str, strings::style st) noexcept {
    bool did_write = false;
    while(str) {
      if (auto line = str.take_prefix(min(str.size(), j::strcspn(str.begin(), "\n")))) {
        did_write |= write(line, st);
      } else if (did_write) {
        if (break_queued && str) {
          queue_empty_line();
        }
      }
      if (str) {
        if (pack_empty_lines) {
          queue_break();
        } else {
          did_write |= newline();
        }
        str.remove_prefix(1U);
      }
    }
    return did_write;
  }

  bool code_writer::write_block_with_line_breaks(strings::const_string_view str) noexcept {
    bool did_write = false;
    while(str) {
      did_write |= write_line(str.take_prefix(min(str.size(), j::strcspn(str.begin(), "\n") + 1U)));
    }
    return did_write;
  }
}
