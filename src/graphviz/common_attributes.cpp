#include "graphviz/common_attributes.hpp"
#include "strings/format.hpp"
#include "strings/string.hpp"

namespace j::graphviz {
  common_attributes::common_attributes() noexcept { }

  bool common_attributes::visit_string_attribute(
    attribute_visitor_t & v,
    const char * key,
    const strings::string & value
  ) {
    if (value.empty()) {
      return false;
    }
    v(key, value, true);
    return true;
  }

  bool common_attributes::visit_string_attribute(
    attribute_visitor_t & v,
    const char * key,
    strings::const_string_view value
  ) {
    if (value.empty()) {
      return false;
    }
    v(key, value, true);
    return true;
  }

  bool common_attributes::visit_float_attribute(
    attribute_visitor_t & v,
    const char * key,
    float value,
    bool check_neg
  ) {
    if (check_neg ? (value < 0.0f) : (value == 0.0f)) {
      return false;
    }
    v(key, strings::format("{}", value), false);
    return true;
  }

  bool common_attributes::visit_int_attribute(
    attribute_visitor_t & v,
    const char * key,
    i32_t value,
    bool check_neg
  ) {
    if (check_neg ? (value < 0) : (value == 0)) {
      return false;
    }
    v(key, strings::format("{}", value), false);
    return true;
  }

  bool common_attributes::visit_color_attribute(attribute_visitor_t & v, const char * key, color value) {
    if (!value) {
      return false;
    }
    v(key, value.format(), true);
    return true;
  }

  bool common_attributes::visit_style_attribute(attribute_visitor_t & v, a::style style) {
    switch (style) {
    case a::style::no_value:
      return false;
    case a::style::filled:
      v("style", "filled", false);
      return true;
    case a::style::striped:
      v("style", "striped", false);
      return true;
    case a::style::solid:
      v("style", "solid", false);
      return true;
    case a::style::dashed:
      v("style", "dashed", false);
      return true;
    case a::style::dotted:
      v("style", "dotted", false);
      return true;
    case a::style::bold:
      v("style", "bold", false);
      return true;
    case a::style::invis:
      v("style", "invis", false);
      return true;
    }
  }

  bool common_attributes::visit_attributes(attribute_visitor_t && v) const {
    switch (label_align) {
    case a::align::no_value: break;
    case a::align::top:
      v("labelloc", "t", false);
      break;
    case a::align::center:
      v("labelloc", "c", false);
      break;
    case a::align::bottom:
      v("labelloc", "b", false);
      break;
    }
    v("label", label, !is_label_html);
    visit_string_attribute(v, "id", id);
    visit_color_attribute(v, "fontcolor", font_color);
    visit_float_attribute(v, "fontsize", font_size);
    visit_string_attribute(v, "fontname", font_name);
    visit_string_attribute(v, "class", css_class);
    return true;
  }
}
