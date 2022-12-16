#pragma once

#include "graphviz/common_attributes.hpp"

namespace j::graphviz {
  namespace a = j::graphviz::attributes;

  /// Attributes applicable to graphs.
  struct graph_attributes : public common_attributes {
    graph_attributes() noexcept;

    template<typename... Args>
    J_A(AI,ND) inline explicit graph_attributes(Args && ... args)
      : common_attributes(static_cast<Args &&>(args)...),
        is_new_rank(j::attributes::has<Args...>(a::new_rank))
    {
      if constexpr (j::attributes::has<Args...>(a::rank)) {
        rank = a::rank.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::ranksep)) {
        ranksep = a::ranksep.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::ranksep_equally)) {
        ranksep_equally = true;
      }
      if constexpr (j::attributes::has<Args...>(a::concentrate)) {
        concentrate = true;
      }
      if constexpr (j::attributes::has<Args...>(a::nodesep)) {
        nodesep = a::nodesep.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::margin)) {
        margin = a::margin.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::graph_style)) {
        style = a::graph_style.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::rank_direction)) {
        rank_direction = a::rank_direction.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::ordering)) {
        ordering = a::ordering.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::output_order)) {
        output_order = a::output_order.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::splines)) {
        splines = a::splines.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::label_justify)) {
        label_justify = a::label_justify.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::foreground_color)) {
        foreground_color = a::foreground_color.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::background_color)) {
        background_color = a::background_color.get(static_cast<Args &&>(args)...);
      }
    }

    bool visit_attributes(attribute_visitor_t && v) const override;

    float ranksep = -1.0f;
    float nodesep = -1.0f;
    strings::string foreground_color_list;
    color foreground_color;
    color background_color;
    float margin = -1.0f;
    a::output_mode output_order:4 = a::output_mode::no_value;
    a::order ordering:4 = a::order::no_value;
    a::rank_dir rank_direction:4 = a::rank_dir::no_value;
    a::style style:4 = a::style::no_value;
    a::spline_type splines:4 = a::spline_type::no_value;
    a::justify label_justify:2 = a::justify::no_value;
    a::rank_type rank:3 = a::rank_type::no_value;
    bool is_new_rank:1 = false;
    bool concentrate:1 = false;
    bool ranksep_equally:1 = false;
  };
}
