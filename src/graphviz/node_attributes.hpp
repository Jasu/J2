#pragma once

#include "graphviz/common_attributes.hpp"

namespace j::graphviz {
  namespace a = j::graphviz::attributes;

  /// Attributes applicable to nodes.
  struct node_attributes final : public common_attributes {
    node_attributes() noexcept;

    template<typename... Args>
    J_A(AI,ND) inline explicit node_attributes(Args && ... args) : common_attributes(static_cast<Args &&>(args)...) {
      namespace aa = j::attributes;
      if constexpr (aa::has<Args...>(a::tooltip)) {
        tooltip = a::tooltip.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::margin)) {
        margin = a::margin.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::width)) {
        width = a::width.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::height)) {
        height = a::height.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::node_style)) {
        style = a::node_style.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::shape)) {
        shape = a::shape.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::outline_color)) {
        outline_color = a::outline_color.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::fill_color)) {
        fill_color = a::fill_color.get(static_cast<Args &&>(args)...);
      }
    }

    bool visit_attributes(attribute_visitor_t && v) const override;
    color fill_color;
    color outline_color;
    float margin = -1.0f;
    float width = -1.0f;
    float height = -1.0f;
    a::node_shape shape = a::node_shape::no_value;
    a::style style:4 = a::style::no_value;
    a::order ordering:2 = a::order::no_value;
    strings::string tooltip;
  };
}
