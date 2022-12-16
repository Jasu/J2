#pragma once

#include "strings/tables/attributes.hpp"
#include "strings/formatting/pad.hpp"
#include "strings/styling/styled_string.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/enable_if_attributes.hpp"

namespace j::strings::inline tables {
  struct table_cell final {
    styled_string content;
    formatting::pad_settings padding;
    u16_t pad_top = 0, pad_bottom = 0;
    u16_t pad_height_to = 0;
    border_style border_style_top:4    = border_style::table_default;
    border_style border_style_bottom:4 = border_style::table_default;
    border_style border_style_left:4   = border_style::table_default;
    border_style border_style_right:4  = border_style::table_default;

    J_ALWAYS_INLINE constexpr table_cell() noexcept = default;

    J_ALWAYS_INLINE table_cell(const char * J_NOT_NULL content) noexcept
      : content{content, {}}
    { }

    J_ALWAYS_INLINE table_cell(styled_string && content) noexcept
      : content(static_cast<styled_string &&>(content))
    { }

    template<typename Str, typename... Attrs, typename = j::attributes::enable_if_attributes_t<Attrs...>>
    J_ALWAYS_INLINE table_cell(Str && content, Attrs && ... attrs)
      : content{static_cast<Str &&>(content)}
    {
      namespace a = j::attributes;
      namespace t = j::strings::tables;

      if constexpr (a::has<Attrs...>(border)) {
        border_style_top = border_style_bottom = border_style_left = border_style_right
          = border.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_top)) {
        border_style_top = border_top.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_bottom)) {
        border_style_bottom = border_bottom.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_left)) {
        border_style_left = border_left.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(border_right)) {
        border_style_right = border_right.get(static_cast<Attrs &&>(attrs)...);
      }

      if constexpr (a::has<Attrs...>(t::pad)) {
        padding = t::pad.get(static_cast<Attrs &&>(attrs)...);
      }

      if constexpr (a::has<Attrs...>(t::pad_top)) {
        pad_top = t::pad_top.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (a::has<Attrs...>(t::pad_bottom)) {
        pad_bottom = t::pad_bottom.get(static_cast<Attrs &&>(attrs)...);
      }

      if constexpr (a::has<Attrs...>(t::pad_height_to)) {
        pad_height_to = t::pad_height_to.get(static_cast<Attrs &&>(attrs)...);
      }
    }
  };
}
