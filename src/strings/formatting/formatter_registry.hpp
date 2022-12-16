#pragma once

#include "strings/formatting/format_value.hpp"

namespace j::strings {
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
  inline namespace formatting {
    class formatter;

    void register_formatter(
      type_id::type_id type,
      const char * name,
      bool is_default,
      const formatter * formatter);
    void unregister_formatter(
      type_id::type_id type,
      const char * name,
      bool is_default);

    const formatter * do_maybe_get_formatter(type_id::type_id type);
    const formatter * do_maybe_get_formatter(type_id::type_id type, const_string_view name);

    J_A(AI,NODISC) inline const formatter * maybe_get_formatter(const format_value & value) {
      return do_maybe_get_formatter(value.type());
    }
    J_A(AI,NODISC) inline const formatter * maybe_get_formatter(const format_value & value, const_string_view name) {
      return name ? do_maybe_get_formatter(value.type(), name) : do_maybe_get_formatter(value.type());
    }
  }
}
