#include "strings/formatting/sink_formatter.hpp"

#include "strings/formatting/format_string_iterator.hpp"
#include "strings/formatting/format_placeholder.hpp"
#include "strings/formatting/formatter_registry.hpp"
#include "strings/formatting/placeholder.hpp"
#include "strings/formatting/parse_push_style.hpp"

#include "strings/styling/styled_sink.hpp"
#include "strings/styling/style.hpp"
#include "streams/sink.hpp"

namespace j::strings {
  inline namespace formatting {
    const placeholder g_empty_placeholder{"{}"};

    u32_t sink_formatter::format(styled_sink & sink,
                                const const_string_view & format_string,
                                const u32_t num_params,
                                const format_value * const params) noexcept
    {
      return format(sink, format_string, num_params, params, g_empty_style);
    }

    u32_t sink_formatter::format(styled_sink & sink,
                                 const const_string_view & format_string,
                                 const u32_t num_params,
                                 const format_value * const params,
                                 style current_style) noexcept {
      u32_t written = 0U;
      style *style_stack = (style*)J_ALLOCA(sizeof(style) * 8U);
      style_stack[0] = current_style;
      style *cur_style = style_stack;
      const style *set_style = nullptr;

      u32_t current_param = 0;
      bool is_first = true;
      for (format_string_iterator it(format_string); (bool)it; it.next(), is_first = false) {
        const_string_view cur = it.current();

        switch (it.token_type()) {
        case format_string_token::style_push:
          cur_style[1] = *cur_style;
          parse_push_style(cur, *++cur_style, params, current_param);
          if (set_style == cur_style) {
            set_style = nullptr;
          }
          break;
        case format_string_token::style_pop:
          --cur_style;
          break;
        case format_string_token::plain_text:
          if (cur_style != set_style) {
            sink.set_style(*cur_style);
            set_style = cur_style;
          }
          written += sink.write(cur.begin(), cur.size());
          break;
        case format_string_token::placeholder:
          if (J_LIKELY(current_param < num_params)) {
            placeholder ph{cur};
            const formatter * f = maybe_get_formatter(params[current_param], ph.formatter_name);
            written += write_placeholder(sink, f, params[current_param], ph, cur_style, &set_style);
            ++current_param;
          } else {
            if (cur_style != set_style) {
              sink.set_style(*cur_style);
              set_style = cur_style;
            }
            written += sink.write("{UNKNOWN}", 9U);
          }
          break;
        case format_string_token::end:
          J_UNREACHABLE();
        }
      }

      is_first = true;
      for (; current_param < num_params; ++current_param) {
        written += sink.write(is_first ? ": " : ", ", 2U);
        const formatter * f = maybe_get_formatter(params[current_param]);
        written += write_placeholder(sink, f, params[current_param], g_empty_placeholder, cur_style, &set_style);
        is_first = false;
      }
      if (set_style != cur_style) {
        sink.set_style(*cur_style);
      }
      return written;
    }

    u32_t sink_formatter::write(styled_sink & sink, const_string_view value) noexcept {
      return this->format(sink, value, 0U, nullptr);
    }

    u32_t sink_formatter::write(styled_sink & sink, const char * value) noexcept {
      return this->format(sink, value, 0U, nullptr);
    }

    u32_t sink_formatter::format_single(styled_sink & sink, const format_value & param, style current_style, const placeholder & ph) noexcept {
      const formatter * f = maybe_get_formatter(param, ph.formatter_name);
      const style *cur_style = &current_style;
      return write_placeholder(sink, f, param, ph, cur_style, &cur_style);
    }
  }
}
