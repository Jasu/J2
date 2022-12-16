#include "graphviz/node_attributes.hpp"
#include "strings/string.hpp"
#include "graphviz/node.hpp"

namespace j::graphviz {
  node::node() noexcept { }
  node_attributes::node_attributes() noexcept {
  }

  bool node_attributes::visit_attributes(attribute_visitor_t && v) const {
    bool result = common_attributes::visit_attributes(static_cast<attribute_visitor_t &&>(v));
    result |= visit_string_attribute(v, "tooltip", tooltip);
    result |= visit_float_attribute(v, "width", width, true);
    result |= visit_float_attribute(v, "height", height, true);
    result |= visit_float_attribute(v, "margin", margin, true);
    result |= visit_style_attribute(v, style);
    result |= visit_color_attribute(v, "color", outline_color);
    result |= visit_color_attribute(v, "fillcolor", fill_color);

    switch (shape) {
    case a::node_shape::no_value:
      break;
    case a::node_shape::plain:
      v("shape", "plain", false);
      result = true;
      break;
    case a::node_shape::plaintext:
      v("shape", "plaintext", false);
      result = true;
      break;
    case a::node_shape::ellipse:
      v("shape", "ellipse", false);
      result = true;
      break;
    case a::node_shape::none:
      v("shape", "none", false);
      result = true;
      break;
    case a::node_shape::doublecircle:
      v("shape", "doublecircle", false);
      result = true;
      break;
    case a::node_shape::oval:
      v("shape", "oval", false);
      result = true;
      break;
    case a::node_shape::circle:
      v("shape", "circle", false);
      result = true;
      break;
    case a::node_shape::egg:
      v("shape", "egg", false);
      result = true;
      break;
    case a::node_shape::triangle:
      v("shape", "triangle", false);
      result = true;
      break;
    case a::node_shape::box:
      v("shape", "box", false);
      result = true;
      break;
    case a::node_shape::diamond:
      v("shape", "diamond", false);
      result = true;
      break;
    case a::node_shape::trapezium:
      v("shape", "trapezium", false);
      result = true;
      break;
    case a::node_shape::parallelogram:
      v("shape", "parallelogram", false);
      result = true;
      break;
    case a::node_shape::house:
      v("shape", "house", false);
      result = true;
      break;
    case a::node_shape::hexagon:
      v("shape", "hexagon", false);
      result = true;
      break;
    case a::node_shape::octagon:
      v("shape", "octagon", false);
      result = true;
      break;
    case a::node_shape::note:
      v("shape", "note", false);
      result = true;
      break;
    case a::node_shape::tab:
      v("shape", "tab", false);
      result = true;
      break;
    case a::node_shape::box3d:
      v("shape", "box3d", false);
      result = true;
      break;
    case a::node_shape::component:
      v("shape", "component", false);
      result = true;
      break;
    }
    return result;
  }
}
