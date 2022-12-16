#pragma once

#include "fonts/selection/flags.hpp"
#include "fonts/selection/attributes.hpp"
#include "fonts/font_size.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/trivial_array.hpp"
#include "strings/string.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::strings::string);

namespace j::fonts::selection {
  struct font_specification final {
    J_BOILERPLATE(font_specification, CTOR_CE)

    template<typename... Attributes>
    J_ALWAYS_INLINE_NO_DEBUG explicit font_specification(Attributes && ... attributes)
      : family(::j::attributes::as_trivial_array_copyable_move<strings::string>(
                 j::fonts::selection::family, static_cast<Attributes &&>(attributes)...))
    {

      if constexpr (j::attributes::has<Attributes...>(j::fonts::selection::weight)) {
        weight = j::fonts::selection::weight.get(static_cast<Attributes &&>(attributes)...);
      }
      if constexpr (j::attributes::has<Attributes...>(j::fonts::selection::width)) {
        width = j::fonts::selection::width.get(static_cast<Attributes &&>(attributes)...);
      }
      if constexpr (j::attributes::has<Attributes...>(j::fonts::selection::slant)) {
        slant = j::fonts::selection::slant.get(static_cast<Attributes &&>(attributes)...);
      }
      if constexpr (j::attributes::has<Attributes...>(j::fonts::selection::spacing)) {
        spacing = j::fonts::selection::spacing.get(static_cast<Attributes &&>(attributes)...);
      }
      if constexpr (j::attributes::has<Attributes...>(j::fonts::selection::size)) {
        size = j::fonts::selection::size.get(static_cast<Attributes &&>(attributes)...);
      }
    }

    font_weight_flags weight = font_weight::regular;
    font_width_flags width = font_width::normal;
    font_slant_flags slant = font_slant::roman;
    font_spacing_flags spacing = font_spacing::proportional;

    /// Family names by priority.
    ///
    /// When empty, any font family matches.
    trivial_array_copyable<strings::string> family;

    /// Font size, in pixels or points.
    font_size size;
  };
}
