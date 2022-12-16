#pragma once

#include "fonts/font_set.hpp"
#include "fonts/selection/font_specification.hpp"
#include "attributes/enable_if_attributes.hpp"

namespace j::fonts::selection {
  struct font_specification;

  /// Base class for font selection engines, e.g. FontConfig on Linux.
  class font_selector {
  public:
    virtual font_set select_fonts(const font_specification & spec) const = 0;

    template<typename... Args, typename = attributes::enable_if_attributes_t<Args...>>
    J_INLINE_GETTER_NO_DEBUG font_set select_fonts(Args && ... args) const {
      font_specification spec(static_cast<Args &&>(args)...);
      return select_fonts(spec);
    }

    virtual ~font_selector();
  };
}
