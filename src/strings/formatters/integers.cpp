#include "strings/formatters/integers.hpp"

#include "strings/formatting/formatter.hpp"
#include "strings/formatting/format_digits.hpp"
#include "strings/string_view.hpp"
#include "strings/string_algo.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "exceptions/assert.hpp"

namespace j::strings::formatters {
  namespace {
    enum class base_t : u8_t {
      dec = 0,
      hex,
      hex_upper,
      bin,
    };

    struct J_TYPE_HIDDEN format_args {
      base_t base = base_t::dec;
      bool align_left = false;
      char pad_char = ' ';
      u8_t pad_to = 0U;
    };

    [[nodiscard]] format_args parse_format_args(const_string_view format_options) noexcept {
      format_args result;
      if (!format_options) {
        return result;
      }
      const char *cur = format_options.begin();
      if (*cur == '+') {
        result.align_left = true;
        ++cur;
      }
      while (is_digit(*cur)) {
        result.pad_to = result.pad_to * 10 + *cur++ - '0';
        if (!result.pad_to) {
          result.pad_char = '0';
        }
      }

      switch (*cur) {
      case 'b':
        result.base = base_t::bin;
        ++cur;
        break;
      case 'X':
        result.base = base_t::hex_upper;
        ++cur;
        break;
      case 'x':
        result.base = base_t::hex;
        ++cur;
        break;
      default:
        break;
      }
      J_ASSERT(!*cur || *cur == '}', "Invalid format", format_options);
      return result;
    }

    template<typename Integer>
    [[nodiscard]] u32_t get_integer_digits(format_args opts, Integer i) noexcept {
      switch (opts.base) {
      case base_t::dec:
        return num_decimal_digits(i);
      case base_t::hex:
      case base_t::hex_upper:
        return num_hex_digits(i);
      case base_t::bin:
        return num_binary_digits(i);
      }
    }

    template<typename Integer>
    class integer_formatter final : public formatter_known_length<Integer> {
    public:
      void do_format(
        const const_string_view & format_options,
        const Integer & value,
        styled_sink & target,
        style current_style
      ) const override {
        conditional_t<is_signed_v<Integer>, i64_t, u64_t> val = value;
        if (current_style.foreground.empty() && current_style.background.empty()) {
          style num_style = styling::styles::bright_green;
          if (!value) {
            num_style = styling::styles::bright_cyan;
          } else if (val < 0) {
            num_style = styling::styles::bright_red;
          }
          target.set_style(override_style(current_style, num_style));
        }
        format_args f = parse_format_args(format_options);
        u8_t digits = get_integer_digits(f, val);
        char * buf = target.get_write_buffer(max(digits, f.pad_to));
        j::memset(buf, f.pad_char, f.pad_to);

        i32_t pad = (f.align_left || f.pad_to < digits) ? 0 : f.pad_to - digits;
        if constexpr (is_signed_v<Integer>) {
          if (val < 0) {
            buf[f.pad_char == '0' ? 0 : pad] = '-';
            ++pad;
            --digits;
            val = -val;
          }
        }
        buf += pad;

        switch (f.base) {
        case base_t::dec:
          formatting::format_dec(val, buf, digits);
          break;
        case base_t::hex_upper:
          formatting::format_hex(val, buf, digits, 'A' - 10);
          break;
        case base_t::hex:
          formatting::format_hex(val, buf, digits, 'a' - 10);
          break;
        case base_t::bin: {
          for (u64_t i = 0; i < digits; ++i) {
            *buf++ = (val & (1ULL << (digits - 1 - i))) ? '1' : '0';
          }
          break;
        }
        }
        if (current_style.foreground.empty() && current_style.background.empty()) {
          target.set_style(current_style);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const Integer & value) const noexcept override {
        const format_args f = parse_format_args(format_options);
        return ::j::max(get_integer_digits<Integer>(f, value), f.pad_to);
      }
    };

    J_A(ND, NODESTROY) const integer_formatter<i8_t> i8_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<i16_t> i16_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<i32_t> i32_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<i64_t> i64_t_formatter;

    J_A(ND, NODESTROY) const integer_formatter<u8_t> u8_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<u16_t> u16_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<u32_t> u32_t_formatter;
    J_A(ND, NODESTROY) const integer_formatter<u64_t> u64_t_formatter;
  }

  template<> const formatter_known_length<u64_t> & integer_formatter_v<u64_t> = u64_t_formatter;
  template<> const formatter_known_length<u32_t> & integer_formatter_v<u32_t> = u32_t_formatter;
  template<> const formatter_known_length<u16_t> & integer_formatter_v<u16_t> = u16_t_formatter;
  template<> const formatter_known_length<u8_t> & integer_formatter_v<u8_t> = u8_t_formatter;
  template<> const formatter_known_length<i64_t> & integer_formatter_v<i64_t> = i64_t_formatter;
  template<> const formatter_known_length<i32_t> & integer_formatter_v<i32_t> = i32_t_formatter;
  template<> const formatter_known_length<i16_t> & integer_formatter_v<i16_t> = i16_t_formatter;
  template<> const formatter_known_length<i8_t> & integer_formatter_v<i8_t> = i8_t_formatter;
}
