#pragma once

#include "graphviz/common_attributes.hpp"

namespace j::graphviz {
  namespace attributes {
    inline constexpr j::attributes::attribute_definition weight{
      j::attributes::value_type = type<i8_t>,
      j::attributes::tag = type<struct weight_tag>};
    inline constexpr j::attributes::attribute_definition minlen{
      j::attributes::value_type = type<i8_t>,
      j::attributes::tag = type<struct minlen_tag>};
    inline constexpr j::attributes::attribute_definition constraint{
      j::attributes::value_type = type<bool>,
      j::attributes::tag = type<struct constraint_tag>};
  }
  namespace a = j::graphviz::attributes;


  /// Attributes applicable to edges.
  class edge_attributes final : public common_attributes {
  public:
    edge_attributes() noexcept;

    template<typename... Args>
    J_A(ND,AI,HIDDEN) inline explicit edge_attributes(Args && ... args)
      : common_attributes(static_cast<Args &&>(args)...) {
      if constexpr (j::attributes::has<Args...>(a::edge_style)) {
        style = a::edge_style.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::edge_color)) {
        edge_color = a::edge_color.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::constraint)) {
        constraint = a::constraint.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::arrow_direction)) {
        arrow_direction = a::arrow_direction.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::arrow_head)) {
        arrow_head = a::arrow_head.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::arrow_tail)) {
        arrow_tail = a::arrow_tail.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::clip_type)) {
        clip_type = a::clip_type.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::arrow_size)) {
        arrow_size = a::arrow_size.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::weight)) {
        weight = a::weight.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::minlen)) {
        minlen = a::minlen.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::width)) {
        pen_width = a::width.get(static_cast<Args &&>(args)...);
      }
    }

    bool visit_attributes(attribute_visitor_t && v) const override;

    float pen_width = -1.0f;
    float arrow_size = -1.0f;
    color edge_color{};
    a::style style = a::style::no_value;
    i8_t weight = -1;
    i8_t minlen = -1;
    a::arrow arrow_head = a::arrow::no_value;
    a::arrow arrow_tail = a::arrow::no_value;
    a::direction arrow_direction:4 = a::direction::no_value;
    a::clip clip_type:2 = a::clip::both;
    bool constraint:1 = true;
  };
}
