#pragma once

#include "strings/string_view.hpp"
#include "util/tristate.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::meta {
  struct code_writer_settings final {
    const char * begin_token = nullptr;
    util::tristate pack_empty_lines = nullptr;
    i8_t indent = -1;
    i8_t inner_indent = -1;

    J_A(AI,NODISC,ND) inline constexpr bool operator==(const code_writer_settings &) const = default;
    [[nodiscard]] bool operator<(const code_writer_settings &) const;
  };

  constexpr inline code_writer_settings cpp_comment    {"// "};
  constexpr inline code_writer_settings cpp_doc_comment{"/// "};
  constexpr inline code_writer_settings hash_comment   {"# "};
  constexpr inline code_writer_settings tpl_lines      {"| "};

  struct code_writer_guard;
  struct code_writer_stream_guard;

  struct code_writer final {
    J_A(AI) inline code_writer(strings::formatted_sink * J_NOT_NULL to) noexcept
      : to(to)
    { }

    code_writer(strings::formatted_sink * J_NOT_NULL to, const code_writer_settings settings) noexcept;

    void configure(const code_writer_settings & settings) noexcept;
    [[nodiscard]] code_writer_stream_guard redirect(strings::formatted_sink * to) noexcept;
    [[nodiscard]] code_writer_guard enter(const code_writer_settings & settings) noexcept;

    [[nodiscard]] code_writer_guard begin_indent() noexcept;
    [[nodiscard]] code_writer_guard begin_indent_inner() noexcept;
    [[nodiscard]] code_writer_guard descend_guarded(i8_t indent = -1) noexcept;
    [[nodiscard]] code_writer_guard descend_inner(i8_t indent = -1) noexcept;

    enum code_writer_state : u8_t {
      line_begin,
      indented,
      token,
      mid_line,
    };

    strings::formatted_sink * to = nullptr;

    u8_t indent = 0U;
    u8_t inner_indent = 0U;
    u8_t indent_step = 2U;
    u32_t cur_col = 0U;
    bool pack_empty_lines = true;
    bool break_queued = false;
    bool empty_line_queued = false;
    code_writer_state state = line_begin;
    bool is_between_sections = false;

    const char * begin_token = "";

    bool begin_write() noexcept;
    void begin_line() noexcept;
    void begin_line_no_token() noexcept;
    void begin_line_no_indent() noexcept;
    bool write(strings::const_string_view str, strings::style st = {}) noexcept;
    bool newline() noexcept;
    J_A(AI,ND) inline void queue_break() noexcept { break_queued = true; }
    J_A(AI,ND) inline void queue_empty_line() noexcept { empty_line_queued = true; }
    bool write_line(strings::const_string_view str, strings::style st = {}) noexcept;
    bool write_with_line_breaks(strings::const_string_view str, strings::style st = {}) noexcept;
    bool write_block_with_line_breaks(strings::const_string_view str) noexcept;
    void write_empty_line() noexcept;
    bool line_break() noexcept;
    J_A(AI) inline void flush_empty_line() noexcept {
      if (empty_line_queued) {
        write_empty_line();
      }
    }
    J_A(AI) inline void flush_break() noexcept {
      begin_line_no_indent();
    }

    J_A(AI) inline void clear_empty_line() noexcept {
      break_queued |= empty_line_queued;
      empty_line_queued = false;
    }
    J_A(AI) inline void clear_section_break() noexcept {
      is_between_sections = false;
    }
    J_A(AI) inline void clear_break() noexcept {
      break_queued = empty_line_queued = false;
    }

    J_A(AI) constexpr inline void descend() noexcept {
      indent += indent_step;
    }

    J_A(AI) constexpr inline void ascend() noexcept {
      indent -= indent_step;
    }

    J_A(AI) inline void reset() noexcept {
      is_between_sections = empty_line_queued = break_queued = false;
      state = line_begin;
    }

    template<typename... Args>
    J_A(AI,ND) inline bool write_formatted(strings::const_string_view format, Args && ... args) noexcept {
      if (!format) {
        return false;
      }
      bool did_write = begin_write();
      u32_t num = to->write(format, static_cast<Args &&>(args)...);
      cur_col += num;
      return num || did_write;
    }

    template<typename... Args>
    J_A(AI,ND) inline bool write_line_formatted(strings::const_string_view format, Args && ... args) noexcept {
      if (!format || format == "\n") {
        return line_break();
      }
      begin_line();
      to->write(format, static_cast<Args &&>(args)...);
      if (format.back() != '\n') {
        to->write_unformatted("\n");
      }
      cur_col = 0U;
      state = line_begin;
      return true;
    }
  };

  struct code_writer_guard final {
    code_writer * writer = nullptr;
    const code_writer_settings settings{};

    void exit() {
      if (writer) {
        writer->configure(settings);
        writer = nullptr;
      }
    }

    ~code_writer_guard() {
      if (writer) {
        writer->configure(settings);
      }
    }
  };

  struct code_writer_stream_guard final {
    code_writer * writer = nullptr;
    strings::formatted_sink * old_stream = nullptr;

    void exit() {
      if (writer) {
        writer->to->flush();
        writer->to = old_stream;
        writer = nullptr;
      }
    }

    ~code_writer_stream_guard() {
      exit();
    }
  };

  [[nodiscard]] inline code_writer_stream_guard code_writer::redirect(strings::formatted_sink * to) noexcept {
    code_writer_stream_guard result{this, this->to};
    if (to) {
      this->to->flush();
      this->to = to;
    }
    return result;
  }
}
