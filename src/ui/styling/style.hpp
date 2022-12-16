#pragma once

#include "ui/attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::ui {
  inline namespace styling {
    struct style final {
      J_BOILERPLATE(style, CTOR_CE)

      colors::rgba8 background_color = {0, 0, 0, 0};

      colors::rgba8 border_color_left = {0, 0, 0, 0};
      colors::rgba8 border_color_right = {0, 0, 0, 0};
      colors::rgba8 border_color_top = {0, 0, 0, 0};
      colors::rgba8 border_color_bottom = {0, 0, 0, 0};

      geometry::perimeter_u8 corner_radius = {0, 0, 0, 0};

      J_INLINE_GETTER bool has_background() const noexcept {
        return background_color.a;
      }

      bool has_border() const noexcept {
        return border_color_left.a || border_color_right.a || border_color_top.a || border_color_bottom.a;
      }

      template<typename... Attrs>
      explicit constexpr style(Attrs && ... attrs) noexcept {
        namespace a = ui::attributes;
        namespace ja = j::attributes;
        if constexpr (ja::has<Attrs...>(a::background_color)) {
          background_color = a::background_color.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::border_color)) {
          border_color_left = border_color_right = border_color_top = border_color_bottom
            = a::border_color.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::border_color_left)) {
          border_color_left = a::border_color_left.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::border_color_right)) {
          border_color_right = a::border_color_right.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::border_color_top)) {
          border_color_top = a::border_color_top.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::border_color_bottom)) {
          border_color_bottom = a::border_color_bottom.get(static_cast<Attrs &&>(attrs)...);
        }
        if constexpr (ja::has<Attrs...>(a::corner_radius)) {
          corner_radius = a::corner_radius.get(static_cast<Attrs &&>(attrs)...);
        }
      }
    };
  }
}
