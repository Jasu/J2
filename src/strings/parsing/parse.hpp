#pragma once

#include "type_id/type_id.hpp"
#include "mem/any.hpp"
#include "strings/string.hpp"
#include "strings/parsing/parse_int.hpp"
#include "strings/parsing/parse_float.hpp"

#include <typeinfo>

namespace j::strings {
  inline namespace parsing {
    mem::any parse(type_id::type_id type, const strings::const_string_view & str);

    template<typename T>
    struct parse_fn {
      J_INLINE_GETTER static T parse(const strings::const_string_view & str) {
        if constexpr (j::is_integral_v<T>) {
          return parse_decimal_integer<T>(str.data(), str.size());
        } else if constexpr (j::is_floating_point_v<T>) {
          return parse_float<T>(str.data(), str.size());
        } else {
          return static_cast<T &&>(strings::parse(type_id::type_id(typeid(T)), str).get_unsafe<T>());
        }
      }
    };

    template<>
    struct parse_fn<strings::const_string_view> {
      J_INLINE_GETTER static const strings::const_string_view & parse(const strings::const_string_view & str) noexcept {
        return str;
      }
    };

    template<>
    struct parse_fn<strings::string> {
      J_INLINE_GETTER static strings::string parse(const strings::const_string_view & str) {
        return str;
      }

      J_INLINE_GETTER static const strings::string & parse(const strings::string & str) noexcept {
        return str;
      }
    };

    template<>
    struct parse_fn<bool> {
      J_INLINE_GETTER static bool parse(const strings::const_string_view & str) noexcept {
        return str == "true";
      }
    };
  }
}
