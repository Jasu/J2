#pragma once

#include "graphviz/common.hpp"
#include "strings/string.hpp"
#include "functions/bound_function.hpp"
#include "graphviz/attributes.hpp"
#include "attributes/enable_if_attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::graphviz {
  namespace a = j::graphviz::attributes;

  using attribute_visitor_t J_NO_DEBUG_TYPE = functions::bound_function<void (const char * key, const strings::const_string_view & value, bool is_string)>;

  /// Base class for node, edge, graph, and cluster attributes.
  class common_attributes {
  public:
    common_attributes() noexcept;

    template<typename... Args, typename = j::attributes::enable_if_attributes_t<Args...>>
    J_A(AI,ND,HIDDEN) inline explicit common_attributes(Args && ... args) {
      if constexpr (j::attributes::has<Args...>(a::font_color)) {
        font_color = a::font_color.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::font_size)) {
        font_size = a::font_size.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::font_name)) {
        font_name = a::font_name.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::id)) {
        id = a::id.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::label)) {
        label = a::label.get(static_cast<Args &&>(args)...);
        is_label_html = j::attributes::has<Args...>(a::label_is_html);
      }
      if constexpr (j::attributes::has<Args...>(a::css_class)) {
        css_class = a::css_class.get(static_cast<Args &&>(args)...);
      }
      if constexpr (j::attributes::has<Args...>(a::label_align)) {
        label_align = a::label_align.get(static_cast<Args &&>(args)...);
      }
    }

    virtual bool visit_attributes(attribute_visitor_t && v) const;

    strings::string label;
    strings::string id;
    strings::const_string_view font_name;
    strings::const_string_view css_class;
    float font_size = 0.0f;
    color font_color;
    a::align label_align = a::align::no_value;
    bool is_label_html = false;

  protected:
    static bool visit_string_attribute(attribute_visitor_t & v, const char * key, strings::const_string_view value);
    static bool visit_string_attribute(attribute_visitor_t & v, const char * key, const strings::string & value);
    static bool visit_float_attribute(attribute_visitor_t & v, const char * key, float value, bool check_neg = false);
    static bool visit_int_attribute(attribute_visitor_t & v, const char * key, i32_t value, bool check_neg = false);
    static bool visit_color_attribute(attribute_visitor_t & v, const char * key, color value);
    static bool visit_style_attribute(attribute_visitor_t & v, a::style style);
  };
}
