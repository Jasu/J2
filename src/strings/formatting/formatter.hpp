#pragma once

#include "strings/formatting/context.hpp"
#include "strings/string_view.hpp"
#include "strings/styling/style.hpp"
#include "type_id/type_id.hpp"
#include "hzd/mem.hpp"

#include <typeinfo>

namespace j::strings {
  class string;
  inline namespace styling {
    class styled_sink;
    struct styled_string;
  }

  inline namespace formatting {
    template<typename... Ts>
    inline type_id::type_id * get_type_ids() {
      type_id::type_id * result = ::j::allocate<type_id::type_id>(sizeof...(Ts));
      u32_t i = 0U;
      (::new (result + i++) type_id::type_id(typeid(Ts)), ...);
      return result;
    }

    class formatter {
    protected:
      formatter(bool supports_get_length,
                u32_t num_types,
                type_id::type_id * J_NOT_NULL types,
                const char * name,
                bool is_default = false) noexcept;

      formatter(bool supports_get_length,
                const std::type_info & type,
                const char * name,
                bool is_default = false) noexcept;
    public:
      virtual void format(const const_string_view & format_options,
                          const void * value,
                          styling::styled_sink & target,
                          styling::style current_style) const;

      [[nodiscard]] virtual u32_t get_length(const const_string_view & format_options, const void * value) const noexcept;

      [[nodiscard]] virtual styling::styled_string format(const const_string_view & format_options, const void * value) const;

      ~formatter();

      formatter(formatter &&) = delete;
      formatter(const formatter &) = delete;
      formatter & operator=(formatter &&) = delete;
      formatter & operator=(const formatter &) = delete;
      const bool supports_get_length:1;
    private:
      const bool m_is_default:1;
      const u32_t m_num_types;
      union {
        const type_id::type_id * m_types;
        type_id::type_id m_type;
      };
      const char * m_name = nullptr;
    };

    template<typename T, typename... ExtraTypes>
    class formatter_known_length : public formatter {
    public:
      J_ALWAYS_INLINE_NO_DEBUG explicit formatter_known_length(const char * name = nullptr, bool is_default = true) noexcept
        : formatter(true, 1 + sizeof...(ExtraTypes), get_type_ids<T, ExtraTypes...>(), name, is_default) { }

      virtual void do_format(const const_string_view & format_options,
                             const T & value,
                             styling::styled_sink & target,
                             styling::style current_style) const = 0;

      [[nodiscard]] virtual u32_t do_get_length(const const_string_view & format_options, const T & value) const noexcept = 0;

      J_NO_DEBUG void format(
        const const_string_view & format_options,
        const void * value,
        styling::styled_sink & target,
        styling::style current_style
      ) const override final {
        do_format(format_options,
                  *reinterpret_cast<const T*>(value),
                  target,
                  current_style);
      }

      [[nodiscard]] J_NO_DEBUG u32_t get_length(const const_string_view & format_options,
                       const void * value) const noexcept override final {
        return do_get_length(format_options, *reinterpret_cast<const T*>(value));
      }
    };

    template<typename T>
    class formatter_known_length<T> : public formatter {
    public:
      J_ALWAYS_INLINE_NO_DEBUG explicit formatter_known_length(const char * name = nullptr, bool is_default = true) noexcept
        : formatter(true, typeid(T), name, is_default) { }

      virtual void do_format(const const_string_view & format_options,
                             const T & value,
                             styling::styled_sink & target,
                             styling::style current_style) const = 0;

      [[nodiscard]] virtual u32_t do_get_length(const const_string_view & format_options, const T & value) const noexcept = 0;

      J_NO_DEBUG void format(
        const const_string_view & format_options,
        const void * value,
        styling::styled_sink & target,
        styling::style current_style
      ) const override final {
        do_format(format_options,
                  *reinterpret_cast<const T*>(value),
                  target,
                  current_style);
      }

      [[nodiscard]] J_NO_DEBUG u32_t get_length(const const_string_view & format_options,
                       const void * value) const noexcept override final {
        return do_get_length(format_options, *reinterpret_cast<const T*>(value));
      }
    };
  }
}
