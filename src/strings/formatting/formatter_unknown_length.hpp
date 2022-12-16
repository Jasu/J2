#pragma once

#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_string.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"

namespace j::strings {
  inline namespace formatting {

    template<typename T, typename... ExtraTypes>
    class formatter_unknown_length : public formatter {
    public:
      J_ALWAYS_INLINE_NO_DEBUG explicit formatter_unknown_length(const char * name = nullptr, bool is_default = true) noexcept
        : formatter(false, 1 + sizeof...(ExtraTypes), get_type_ids<T, ExtraTypes...>(), name, is_default) { }

      virtual styling::styled_string do_format(const const_string_view & format_options,
                                               const T & value) const = 0;

      J_NO_DEBUG styling::styled_string format(const const_string_view & format_options,
                                    const void * value) const override final {
        return do_format(format_options, *reinterpret_cast<const T*>(value));
      }
    };

    template<typename T>
    class formatter_unknown_length<T> : public formatter {
    public:
      J_ALWAYS_INLINE_NO_DEBUG explicit formatter_unknown_length(const char * name = nullptr, bool is_default = true) noexcept
        : formatter(false, typeid(T), name, is_default) { }

      virtual styling::styled_string do_format(const const_string_view & format_options,
                                               const T & value) const = 0;

      J_NO_DEBUG styling::styled_string format(const const_string_view & format_options,
                                    const void * value) const override final {
        return do_format(format_options, *reinterpret_cast<const T*>(value));
      }
    };
  }
}

#pragma clang diagnostic pop
