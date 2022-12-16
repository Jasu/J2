#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatting/pad.hpp"
#include "strings/string_algo.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/formatting/format_digits.hpp"
#include "strings/formatters/integers.hpp"

namespace j::strings::formatters {
  namespace {
    struct J_TYPE_HIDDEN format_opts final {
      u8_t pad_to = 0U;
      u8_t pad_left_by = 0U;
      u8_t pad_right_by = 0U;
      bool right_align = false;
      bool no_style = false;
    };

    [[nodiscard]] format_opts parse_format_opts(strings::const_string_view opts) {
      format_opts result;
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
      if (maybe_skip_prefix(opts, '-')) {
        result.no_style = true;
      }
      return result;
    }

  }
  void debug_enum_formatter_base::format(u32_t value, styled_sink & target, const strings::const_string_view & opts) const {
    auto o = parse_format_opts(opts);
    u32_t len = o.pad_left_by + o.pad_right_by;

    if (auto entry = maybe_get_entry(value)) {
      len += ::j::strlen(entry->name);
      write_spaces(target, o.pad_left_by + (o.right_align ? max(0, o.pad_to - len) : 0));
      if (o.no_style) {
        target.write(entry->name);
      } else {
        target.write_styled(entry->style, entry->name);
      }
    } else {
      len += 8U + num_decimal_digits(value);
      write_spaces(target, o.pad_left_by + (o.right_align ? max(0, o.pad_to - len) : 0));
      if (o.no_style) {
        target.write("Unknown#");
      } else {
        target.write_styled(styles::bright_red, "Unknown#");
      }
      integer_formatter_v<u32_t>.do_format("", value, target, styles::bright_red);
    }
    write_spaces(target, o.pad_right_by + (o.right_align ? 0 : max(0, o.pad_to - len)));
  }

  [[nodiscard]] u32_t debug_enum_formatter_base::get_length(u32_t val, const strings::const_string_view & opts) const noexcept {
    auto o = parse_format_opts(opts);
    u32_t len = o.pad_left_by + o.pad_right_by;
    if (const auto e = maybe_get_entry(val)) {
      len += ::j::strlen(e->name);
    } else {
      len += 8U + num_decimal_digits(val);
    }
    return max(len, (u32_t)o.pad_to);
  }
}
