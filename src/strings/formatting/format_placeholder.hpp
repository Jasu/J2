#pragma once

#include "strings/formatting/context.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatting/placeholder.hpp"
#include "strings/formatting/format_value.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/styled_string.hpp"

namespace j::strings::inline formatting {
  inline u32_t get_placeholder_size(const formatter * formatter,
                                    const format_value & param,
                                    const placeholder & ph) {

    const inline_type inl_type = param.get_inline_type();
    const type_id::type_id type = param.type();

    // If the formatter is the default formatter with no parameters, pass strings
    // through directly - i.e. return the length of the string without consulting
    // the formatter (even though formatters for all supported string types exist.)
    if ((ph.formatter_name.empty() && ph.formatter_options.empty()) || !formatter) {
      switch (inl_type) {
      case inline_type::ref:
      case inline_type::ptr:
        if (type == typeid(styled_string)) {
          return param.as<styled_string>()->string.size();
        }
        break;
      case inline_type::string:
        return param.as_string_view().size();
      default:
        break;
      }
    }

    if (J_UNLIKELY(!formatter)) {
      return 9U; // {UNKNOWN}
    } else if (formatter->supports_get_length) {
      return formatter->get_length(ph.formatter_options, param.as_void_star());
    } else {
      // Formatter that does not know the size in advance. Format the strng into a
      // styled_string here and store it in the mem::any (overwriting the original
      // parameter). Note that this means that styled_strings must be passed through
      // when formatting, even when a formatter was used.
      // This also makes it impossible to make a formatter that takes a styled_string
      // as an input (without modifying the method used to pass state between
      // size calculation and formatting.)
      return formatter->format(ph.formatter_options, param.as_void_star()).string.size();
    }
  }

  inline u32_t write_placeholder(
    styled_sink & to,
    const class formatter * formatter,
    const format_value & param,
    const placeholder & ph,
    const style *current_style,
    const style **set_style
    ) {
    // Pass strings through directly, unless formatting options are specified.
    const inline_type inl_type = param.get_inline_type();
    const type_id::type_id type = param.type();
    if ((ph.formatter_name.empty() && ph.formatter_options.empty()) || !formatter) {
      switch (inl_type) {
      case inline_type::ref:
      case inline_type::ptr:
        if (type == typeid(styled_string)) {
          if (current_style != *set_style) {
            *set_style = current_style;
            to.set_style(*current_style);
          }
          return to.write_styled(*param.as<styled_string>());
        }
        break;
      case inline_type::string: {
        auto sv = param.as_string_view();
        if (!sv) {
          return 0;
        }
        if (current_style != *set_style) {
          *set_style = current_style;
          to.set_style(*current_style);
        }
        return to.write(sv);
      }
      default:
        break;
      }
    }


    // Handle styled_string outside the default formatter check here, unlike
    // when getting string length. String formatters that do not know the
    // string length in advance return a styled string directly - that string
    // is stored in the mem::any of that parameter when getting the length.
    // In that case, the styled_string must be returned directly even if a
    // formatter was certainly used.
    //
    // Note that when formatting to a stream, the size checking is never
    // called and the mem::any still has the original argument.
    // if (param.is<styled_string>()) {
    //   for (auto & part : param.get_unsafe<styled_string>()) {
    //     to.set_style(override_style(current_style, part.style()));
    //     to.write(part.string().data(), part.string().size());
    //   }
    //   to.set_style(current_style);
    if (J_UNLIKELY(!formatter)) {
      if (current_style != *set_style) {
        *set_style = current_style;
        to.set_style(*current_style);
      }
      return to.write("{UNKNOWN}", 9U);
    } else if (formatter->supports_get_length) {
      u32_t len = formatter->get_length(ph.formatter_options, param.as_void_star());
      if (len) {
        if (current_style != *set_style) {
          *set_style = current_style;
          to.set_style(*current_style);
        }
        formatter->format(ph.formatter_options, param.as_void_star(), to, *current_style);
      }
      return len;
    } else {
      if (current_style != *set_style) {
        *set_style = current_style;
        to.set_style(*current_style);
      }
      return to.write_styled(formatter->format(ph.formatter_options, param.as_void_star()));
      // // When formatting a string, the formatting is done when getting string
      // // size. When formatting a stream, this case is met.
      // styled_string s = formatter->format(ph.formatter_options(), param.as_void_star());
      // bool had_style = false;
      // for (auto & part : s) {
      //   if (!part.style().empty()) {
      //     had_style = true;
      //     to.set_style(override_style(current_style, part.style()));
      //   }
      //   to.write(part.string().data(), part.string().size());
      // }
      // if (had_style) {
      //   to.set_style(current_style);
      // }
    }
  }
}
