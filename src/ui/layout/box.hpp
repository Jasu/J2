#pragma once

#include "ui/layout/size.hpp"
#include "ui/attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::ui::inline layout {
  /// Definition of the box of an element.
  struct box final {
    J_BOILERPLATE(box, CTOR_CE)

    /// Minimum size required for the content.
    geometry::vec2u16 min_content_size = geometry::vec2u16{0U, 0U};

    /// Maximum size allowed for the content.
    geometry::vec2u16 max_content_size = geometry::vec2u16{U16_MAX, U16_MAX};

    /// Margin around the element.
    ///
    /// The margin starts from the border.
    geometry::perimeter_u8 margin = geometry::perimeter_u8{0U};

    /// Border within the element.
    geometry::perimeter_u8 border = geometry::perimeter_u8{0U};

    template<typename... Attrs>
    J_ALWAYS_INLINE explicit constexpr box(Attrs && ... attrs) noexcept {
      namespace a = ui::attributes;
      namespace ja = j::attributes;
      if constexpr (ja::has<Attrs...>(a::min_size)) {
        min_content_size = a::min_size.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (ja::has<Attrs...>(a::max_size)) {
        max_content_size = a::max_size.get(static_cast<Attrs &&>(attrs)...);
      }
      J_ASSERT(min_content_size.x <= max_content_size.x
                && min_content_size.y <= max_content_size.y,
                "Content size limits out of range.");
      if constexpr (ja::has<Attrs...>(a::margin)) {
        margin = a::margin.get(static_cast<Attrs &&>(attrs)...);
      }
      if constexpr (ja::has<Attrs...>(a::border_size)) {
        border = a::border_size.get(static_cast<Attrs &&>(attrs)...);
      }
    }

    J_INLINE_GETTER constexpr size min_size() const noexcept {
      return { { min_content_size.x, border.left, margin.left, border.right, margin.right },
               { min_content_size.y, border.top, margin.top, border.bottom, margin.bottom }, };
    }

    J_INLINE_GETTER constexpr size max_size() const noexcept {
      return { { max_content_size.x, border.left, margin.left, border.right, margin.right },
               { max_content_size.y, border.top, margin.top, border.bottom, margin.bottom }, };
    }
  };

  /// Box whose size has been determined.
  struct sized_box final {
    /// The determined size of the box.
    geometry::vec2u16 content_size = geometry::vec2u16{0U, 0U};

    /// Margin around the element.
    ///
    /// The margin starts from the border.
    geometry::perimeter_u8 margin = geometry::perimeter_u8{0U};

    /// Border within the element.
    geometry::perimeter_u8 border = geometry::perimeter_u8{0U};
  };
}
