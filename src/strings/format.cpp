#include "strings/format.hpp"
#include "strings/formatting/formatter_registry.hpp"
#include "strings/formatting/format_string_iterator.hpp"
#include "strings/formatting/placeholder.hpp"
#include "strings/formatting/format_placeholder.hpp"
#include "strings/formatting/parse_push_style.hpp"
#include "strings/styling/styled_string_sink.hpp"

namespace j::strings::detail {
  const static placeholder g_empty_placeholder{"{}"};

  namespace {
    [[nodiscard]] u32_t get_result_size(const const_string_view & format_string,
                          u32_t num_params,
                          const formatting::format_value * params,
                          const formatter ** formatters) {
      u32_t result_size = 0, current_param = 0;
      for (format_string_iterator it(format_string); (bool)it; it.next()) {
        switch (it.token_type()) {
        case format_string_token::style_push:
        case format_string_token::style_pop:
          continue;
        case format_string_token::plain_text:
          result_size += it.current().size();
          break;
        case format_string_token::placeholder:
          if (current_param < num_params) {
            placeholder ph{it.current()};
            formatters[current_param] = maybe_get_formatter(params[current_param], ph.formatter_name);
            result_size += get_placeholder_size(formatters[current_param], params[current_param], ph);
            ++current_param;
          } else {
            result_size += 9U; // "{UNKNOWN}"
            formatters[current_param++] = nullptr;
          }
          break;
        case format_string_token::end:
          J_UNREACHABLE();
        }
      }
      while(current_param < num_params) {
        formatters[current_param] = maybe_get_formatter(params[current_param]);
        result_size += 2 + get_placeholder_size(formatters[current_param], params[current_param], g_empty_placeholder);
        ++current_param;
      }
      return result_size;
    }
  }

  [[nodiscard]] styled_string format_styled(const const_string_view & format_string,
                              u32_t num_params,
                              const formatting::format_value * params) {
    const formatter * formatters[num_params];
    style *style_stack = (style*)J_ALLOCA(sizeof(style) * 8U);
    j::memzero(style_stack, sizeof(style));
    style *cur_style = style_stack;
    const style *set_style = style_stack;

    u32_t result_size = get_result_size(format_string, num_params, params, formatters);

    styled_string_sink sink(result_size);
    u32_t current_param = 0;
    for (format_string_iterator it(format_string); (bool)it; it.next()) {
      const_string_view cur = it.current();
      switch (it.token_type()) {
      case format_string_token::style_push:
        cur_style[1] = *cur_style;
        parse_push_style(cur, *++cur_style, params, num_params);
        if (set_style == cur_style) {
          set_style = nullptr;
        }
        break;
      case format_string_token::style_pop:
        --cur_style;
        break;
      case format_string_token::plain_text:
        if (cur_style != set_style) {
          set_style = cur_style;
          sink.set_style(*cur_style);
        }
        sink.write(cur.data(), cur.size());
        break;
      case format_string_token::placeholder:
        if (current_param < num_params) {
          placeholder ph{cur};
          write_placeholder(sink, formatters[current_param], params[current_param], ph, cur_style, &set_style);
          ++current_param;
        } else {
          if (set_style != cur_style) {
            set_style = cur_style;
            sink.set_style(*cur_style);
          }
          sink.write("{UNKNOWN}", 9U);
        }
        break;
      case format_string_token::end:
        J_UNREACHABLE();
      }
    }

    cur_style = style_stack;

    bool is_first = true;
    while (current_param < num_params) {
      sink.write(is_first ? ": " : ", ", 2U);
      write_placeholder(sink, formatters[current_param], params[current_param], g_empty_placeholder, cur_style, &set_style);
      ++current_param;
      is_first = false;
    }
    if (set_style != cur_style) {
      sink.set_style(*cur_style);
    }

    return static_cast<styled_string &&>(sink.string());
  }

  [[nodiscard]] string format(const const_string_view & format_string,
                u32_t num_params,
                const formatting::format_value * params) {
    const formatter * formatters[num_params];
    u32_t result_size = get_result_size(format_string, num_params, params, formatters);
    styled_string_sink sink(result_size);
    u32_t current_param = 0;
    const style * cur_style = &g_empty_style;
    for (format_string_iterator it(format_string); (bool)it; it.next()) {
      const_string_view cur = it.current();
      switch (it.token_type()) {
      case format_string_token::style_push:
      case format_string_token::style_pop:
        break;
      case format_string_token::plain_text:
        sink.write(cur.data(), cur.size());
        break;
      case format_string_token::placeholder:
        if (current_param < num_params) {
          placeholder ph{cur};
          write_placeholder(sink, formatters[current_param], params[current_param], ph, cur_style, &cur_style);
          ++current_param;
        } else {
          sink.write("{UNKNOWN}", 9U);
        }
        break;
      case format_string_token::end:
        J_UNREACHABLE();
      }
    }

    bool is_first = true;
    while (current_param < num_params) {
      sink.write(is_first ? ": " : ", ", 2U);
      write_placeholder(sink, formatters[current_param], params[current_param], g_empty_placeholder, cur_style, &cur_style);
      ++current_param;
      is_first = false;
    }

    return static_cast<string &&>(sink.string().string);
  }
}
namespace j::strings {
  [[nodiscard]] styled_string format_styled(const const_string_view & format_string) {
    return detail::format_styled(format_string, 0U, nullptr);
  }

  [[nodiscard]] string format(const const_string_view & format_string) {
    return detail::format(format_string, 0U, nullptr);
  }

  [[nodiscard]] string format(const const_string_view & format_string, u8_t param) {
    return format<const u64_t &>(format_string, param);
  }

  [[nodiscard]] string format(const const_string_view & format_string, u16_t param) {
    return format<const u64_t &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, u32_t param) {
    return format<const u64_t &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, u64_t param) {
    return format<const u64_t &>(format_string, param);
  }

  [[nodiscard]] string format(const const_string_view & format_string, i8_t param) {
    return format<const i64_t &>(format_string, param);
  }

  [[nodiscard]] string format(const const_string_view & format_string, i16_t param) {
    return format<const i64_t &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, i32_t param) {
    return format<const i64_t &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, i64_t param) {
    return format<const i64_t &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, const_string_view param) {
    return format<const const_string_view &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, const string & param) {
    return format<const const_string_view &>(format_string, param);
  }
  [[nodiscard]] string format(const const_string_view & format_string, const char * param) {
    return format<const const_string_view &>(format_string, param);
  }
}
