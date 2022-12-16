#pragma once

#include "strings/formatters/enum_value.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/parsing/parser.hpp"
#include "containers/hash_map_fwd.hpp"
#include "containers/span.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "attributes/attribute_definition.hpp"
#include "attributes/enable_if_attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::strings::formatters::detail {
  struct enum_value_details final {
    const_string_view name;
    style style;
  };

  struct enum_value_detail_set final {
    enum_value_details visual;
    enum_value_details visual_short;
    enum_value_details serialization;

    const enum_value_details & get_values(context_type t) const noexcept;
  };
}

J_DECLARE_EXTERN_HASH_MAP(u64_t, j::strings::formatters::detail::enum_value_detail_set);
J_DECLARE_EXTERN_UNSORTED_STRING_MAP(u64_t);

namespace j::strings::formatters {
  namespace a = j::attributes;
  using enum_value_span = span<const enum_value>;
  using enum_map_t = hash_map<u64_t, detail::enum_value_detail_set>;

  constexpr inline a::attribute_definition visual{
    a::tag = type<struct visual_tag>,
    a::value_type = type<enum_value_span>
  };

  constexpr inline a::attribute_definition visual_short{
    a::tag = type<struct visual_short_tag>,
    a::value_type = type<enum_value_span>
  };

  constexpr inline a::attribute_definition serialization{
    a::tag = type<struct serialization_tag>,
    a::value_type = type<enum_value_span>
  };

  enum class formatter_type : u8_t {
    normal,
    flags_mask,
    flags_shift,
  };

  namespace detail {
    class enum_formatter_base {
    public:
      constexpr enum_formatter_base() noexcept = default;
      explicit enum_formatter_base(u32_t num);
      enum_formatter_base(enum_value_span values);
      void add_value(const enum_value & value);
    protected:

      void format(
        const const_string_view & format_options,
        u64_t value,
        styled_sink & target,
        style current_style,
        formatter_type type
      ) const;

      [[nodiscard]] u32_t get_length(
        const const_string_view & format_options,
        const u64_t & value, formatter_type type) const noexcept;
      [[nodiscard]] u64_t parse(const const_string_view & str) const;

      void initialize_visual(enum_value_span values);
      void initialize_visual_short(enum_value_span values);
      void initialize_serialization(enum_value_span values);
    private:
      enum_map_t m_enums;
      uncopyable_unsorted_string_map<u64_t> m_values_by_name;
    };
  }

  template<typename Enum, formatter_type Type = formatter_type::normal>
  class enum_formatter
    : public formatter_known_length<Enum>,
      public parsing::parser<Enum>,
      public detail::enum_formatter_base
  {
    static_assert(j::is_enum_v<Enum>);
  public:
    using detail::enum_formatter_base::enum_formatter_base;

    template<u32_t I>
    J_ALWAYS_INLINE_NO_DEBUG explicit enum_formatter(const enum_value (& values)[I])
      : detail::enum_formatter_base(enum_value_span(values))
    { }

    J_ALWAYS_INLINE_NO_DEBUG explicit enum_formatter(enum_value_span values)
      : detail::enum_formatter_base(values)
    { }

    template<typename... Args, typename = a::enable_if_attributes_t<Args...>>
    J_ALWAYS_INLINE_NO_DEBUG explicit enum_formatter(Args && ... args) {
      if constexpr (a::has<Args...>(serialization)) {
        initialize_serialization(serialization.get(static_cast<Args &&>(args)...));
      }
      if constexpr (a::has<Args...>(visual)) {
        initialize_visual(visual.get(static_cast<Args &&>(args)...));
      }
      if constexpr (a::has<Args...>(visual_short)) {
        initialize_visual_short(visual_short.get(static_cast<Args &&>(args)...));
      }
    }

    J_NO_DEBUG void do_format(
      const const_string_view & format_options,
      const Enum & value,
      styled_sink & target,
      style current_style
    ) const override {
      format(format_options, static_cast<u64_t>(value), target, current_style, Type);
    }

    [[nodiscard]] J_NO_DEBUG Enum do_parse(const const_string_view & str) const override {
      return static_cast<Enum>(parse(str));
    }

    [[nodiscard]] J_NO_DEBUG u32_t do_get_length(
      const const_string_view & format_options,
      const Enum & value) const noexcept override {
      return get_length(format_options, static_cast<u64_t>(value), Type);
    }
  };

  template<typename Enum>
  using flags_formatter_mask J_NO_DEBUG_TYPE = enum_formatter<Enum, formatter_type::flags_mask>;

  template<typename Enum>
  using flags_formatter_shift J_NO_DEBUG_TYPE = enum_formatter<Enum, formatter_type::flags_shift>;
}
