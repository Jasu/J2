#include "strings/formatters/strings.hpp"
#include "strings/unicode/rope/rope_utf8_view.hpp"
#include "strings/formatters/string_format_options.hpp"
#include "strings/formatting/format_escaped.hpp"
#include "strings/string.hpp"
#include "strings/string_algo.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::strings::formatters {
  namespace {
    const char ellipsis[] = "…";
    const char mid_ellipsis[] = "⋯";
    constexpr u32_t ellipsis_size = sizeof(ellipsis) - 1U;
    static_assert(sizeof(ellipsis) == sizeof(mid_ellipsis));

    const char padding[] = "                                                                "
                           "                                                                "
                           "                                                                "
                           "                                                                ";
    static_assert(sizeof(padding) == 257);
  }

  [[nodiscard]] string_format_options parse_string_format_options(strings::const_string_view opts) {
    string_format_options result;
    if (maybe_skip_prefix(opts, "...")) {
      if (maybe_skip_prefix(opts, '-')) {
        result.truncate_mode = truncate_mode_t::left;
      } else if (maybe_skip_prefix(opts, '=')) {
        result.truncate_mode = truncate_mode_t::center;
      }
      result.truncate_to = take_integer<u8_t>(opts);
      maybe_skip_prefix(opts, ',');
    }

    if (!opts) { return result; }

    if (maybe_skip_prefix(opts, '>')) {
      result.right_align = true;
    } else {
      maybe_skip_prefix(opts, '<');
    }
    J_ASSERT_NOT_NULL(opts);

    if (is_digit(opts.front())) {
      result.pad_to = take_integer<u8_t>(opts);
      if (!opts) { return result; }
    }
    if (maybe_skip_prefix(opts, '+')) {
      result.pad_left_by = take_integer<u8_t>(opts);
      if (!opts) {
        result.pad_right_by = result.pad_left_by;
        return result;
      }
      if (maybe_skip_prefix(opts, ',')) {
        result.pad_right_by = take_integer<u8_t>(opts);
        if (!opts) { return result; }
      }
    }

    skip_prefix(opts, '$');
    if (maybe_skip_prefix(opts, 'u')) {
      result.escape_options = &unicode_escape_options;
    } else {
      result.escape_options = &default_escape_options;
    }
    J_ASSERT(!opts, "Trailing format characters");
    return result;
  }

  [[nodiscard]] i32_t escaped_size(const string_format_options & opts, const_string_view str) noexcept {
    return opts.escape_options ? count_escaped_bytes(str, *opts.escape_options) : str.size();
  }

  [[nodiscard]] i32_t compute_string_size(const string_format_options & opts, i32_t length) {
    if (opts.truncate_to && length > opts.truncate_to) {
      length = opts.truncate_to - 1 + ellipsis_size;
    }
    return opts.pad_left_by + opts.pad_right_by + j::max(opts.pad_to, length);
  }

  [[nodiscard]] i32_t compute_string_left_size(const string_format_options & opts, i32_t length) noexcept {
    if (!opts.truncate_to || length <= opts.truncate_to) {
      return length;
    }
    switch (opts.truncate_mode) {
    case truncate_mode_t::right:
      return opts.truncate_to - 1;
    case truncate_mode_t::left:
      return 0;
    case truncate_mode_t::center:
      return opts.truncate_to >> 1;
    }
  }
  i32_t write_escaped(styled_sink & target, const string_format_options & opts, const_string_view str, i32_t sz, bool is_left) {
    if (!str || !sz) {
      return 0;
    }
    str = is_left ? str.prefix(::j::min(str.size(), sz)) : str.suffix(::j::min(str.size(), sz));
    if (!opts.escape_options) {
      target.write(str.data(), str.size());
      return str.size();
    }
    const i32_t escaped_size = count_escaped_bytes(str, *opts.escape_options);
    i32_t written = 0U;
    J_ASSERT(escaped_size >= str.size(), "Escaped size out of range.");
    i32_t skip = is_left ? 0U : escaped_size - sz;
    iterate_escaped(str, [&](escape_type t, const_string_view str) {
      J_ASSERT_NOT_NULL(str);
      if (!sz) {
        return;
      }
      if (skip) {
        if (str.size() <= skip) {
          skip -= str.size();
          return;
        } else {
          str.remove_prefix(skip);
          skip = 0U;
        }
      }

      if (sz < str.size()) {
        str = str.prefix(sz);
        sz = 0U;
      } else {
        sz -= str.size();
      }

      written += str.size();
      if (t == escape_type::none) {
        target.write(str.data(), str.size());
      } else {
        style s;
        switch (t) {
        case escape_type::unicode: s = styles::bright_green; break;
        case escape_type::hex: s = styles::bright_red; break;
        case escape_type::special: s = styles::bright_cyan; break;
        case escape_type::none: J_UNREACHABLE();
        }
        target.write_styled(s, str.data(), str.size());
      }
    }, *opts.escape_options);
    return written;
  }

  [[nodiscard]] i32_t compute_string_right_size(const string_format_options & opts, i32_t length) noexcept {
    if (!opts.truncate_to || length <= opts.truncate_to) {
      return 0;
    }
    switch (opts.truncate_mode) {
    case truncate_mode_t::right:
      return 0;
    case truncate_mode_t::left:
      return opts.truncate_to - 1;
    case truncate_mode_t::center:
      return (opts.truncate_to - 1) >> 1;
    }
  }

  void write_string_ellipsis(styled_sink & target, string_format_options opts, u32_t length) {
    if (!opts.truncate_to || length <= opts.truncate_to) {
      return;
    }
    target.write_styled(styles::bright_red, opts.truncate_mode == truncate_mode_t::center ? mid_ellipsis : ellipsis, ellipsis_size);
  }

  void write_string_padding(styled_sink & target, string_format_options opts, u32_t length, bool is_right) {
    if (opts.truncate_to) {
      length = ::j::min(length, opts.truncate_to);
    }
    u8_t pad = opts.pad_left_by;
    if (length < opts.pad_to && opts.right_align == is_right) {
      pad += opts.pad_to - length;
    }
    if (pad) {
      target.write(padding, pad);
    }
  }

  namespace {
    template<typename Str>
    class string_formatter final : public formatter_known_length<Str> {
    public:
      void do_format(
        const const_string_view & format_options,
        const Str & value,
        styled_sink & target,
        style
      ) const override {
        const const_string_view v(value);
        const auto opts = parse_string_format_options(format_options);
        const i32_t len = escaped_size(opts, v);
        J_ASSERT(len >= v.size(), "Escaped size out of range.");
        write_string_padding(target, opts, len, false);
        const i32_t left_size = compute_string_left_size(opts, len);
        [[maybe_unused]] const i32_t left_written = write_escaped(target, opts, v, left_size, true);
        J_ASSERT(left_written == left_size, "Left write out of range.");
        write_string_ellipsis(target, opts, len);
        const i32_t right_size = compute_string_right_size(opts, len);
        [[maybe_unused]] const i32_t right_written = write_escaped(target, opts, v, right_size, false);
        J_ASSERT(right_written == right_size, "Right write out of range.");
        write_string_padding(target, opts, len, true);
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const Str & value
      ) const noexcept override {
        const string_format_options opts = parse_string_format_options(format_options);
        return compute_string_size(opts, escaped_size(opts, const_string_view(value)));
      }
    };

    J_A(ND, NODESTROY) const string_formatter<string_view> string_view_formatter;
    J_A(ND, NODESTROY) const string_formatter<const_string_view> const_string_view_formatter;
    J_A(ND, NODESTROY) const string_formatter<utf8_string_view> utf8_string_view_formatter;
    J_A(ND, NODESTROY) const string_formatter<const_utf8_string_view> const_utf8_string_view_formatter;
    J_A(ND, NODESTROY) const string_formatter<string> string_formatter_impl;
    J_A(ND, NODESTROY) const string_formatter<const char *> const_char_formatter;


    class rope_utf8_view_formatter final : public formatter_known_length<basic_rope_utf8_view<true>, basic_rope_utf8_view<false>> {
    public:
      void do_format(
        const const_string_view &,
        const basic_rope_utf8_view<true> & v,
        styled_sink & to,
        style
      ) const override {
        to.write("\"");
        v.for_each_chunk([&](auto sv) {
          to.write(sv.data(), sv.size());
        });
        to.write("\"");
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view &,
        const basic_rope_utf8_view<true> & v
      ) const noexcept override {
        return 2U + v.size();
      }
    };
    J_A(ND, NODESTROY) const rope_utf8_view_formatter rope_view_formatter;
  }

  constinit const formatter_known_length<const_string_view> & g_const_string_view_formatter = const_string_view_formatter;
  constinit const formatter_known_length<string_view> & g_string_view_formatter = string_view_formatter;
  constinit const formatter_known_length<const_utf8_string_view> & g_const_utf8_string_view_formatter = const_utf8_string_view_formatter;
  constinit const formatter_known_length<utf8_string_view> & g_utf8_string_view_formatter = utf8_string_view_formatter;
  constinit const formatter_known_length<string> & g_string_formatter = string_formatter_impl;
  constinit const formatter_known_length<const char *> & g_const_char_formatter = const_char_formatter;
}
