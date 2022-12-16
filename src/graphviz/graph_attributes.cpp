#include "graphviz/graph_attributes.hpp"
#include "strings/format.hpp"
#include "strings/string.hpp"

namespace j::graphviz {
  graph_attributes::graph_attributes() noexcept {
  }

  bool graph_attributes::visit_attributes(attribute_visitor_t && v) const {
    bool result = common_attributes::visit_attributes(static_cast<attribute_visitor_t &&>(v));
    result |= visit_float_attribute(v, "margin", margin, true);
    if (ranksep_equally) {
      v("ranksep", "equally", false);
      result = true;
    } else {
      result |= visit_float_attribute(v, "ranksep", ranksep, true);
    }
    result |= visit_float_attribute(v, "nodesep", nodesep, true);
    if (is_new_rank) {
      v("newrank", "true", false);
      result = true;
    }
    if (concentrate) {
      v("concentrate", "true", false);
      result = true;
    }
    switch (ordering) {
    case a::order::no_value:
      break;
    case a::order::out:
      v("ordering", "\"out\"", false);
      result = true;
      break;
    case a::order::in:
      v("ordering", "\"in\"", false);
      result = true;
      break;
    }

    switch (output_order) {
    case a::output_mode::no_value:
      break;
    case a::output_mode::breadth_first:
      v("outputorder", "\"node\"", false);
      result = true;
      break;
    case a::output_mode::nodes_first:
      v("outputorder", "\"nodesfirst\"", false);
      result = true;
      break;
    case a::output_mode::edges_first:
      v("outputorder", "\"edgesfirst\"", false);
      result = true;
      break;
    }

    switch (rank) {
    case a::rank_type::no_value:
      break;
    case a::rank_type::same:
      v("rank", "same", false);
      result = true;
      break;
    case a::rank_type::min:
      v("rank", "min", false);
      result = true;
      break;
    case a::rank_type::max:
      v("rank", "max", false);
      result = true;
      break;
    case a::rank_type::source:
      v("rank", "source", false);
      result = true;
      break;
    case a::rank_type::sink:
      v("rank", "sink", false);
      result = true;
      break;
    }

    switch (rank_direction) {
    case a::rank_dir::no_value:
      break;
    case a::rank_dir::top_to_bottom:
      v("rankdir", "TB", false);
      result = true;
      break;
    case a::rank_dir::left_to_right:
      v("rankdir", "LR", false);
      result = true;
      break;
    case a::rank_dir::bottom_to_top:
      v("rankdir", "BT", false);
      result = true;
      break;
    case a::rank_dir::right_to_left:
      v("rankdir", "RL", false);
      result = true;
      break;
    }

    switch (label_justify) {
    case a::justify::no_value:
      break;
    case a::justify::left:
      v("labeljust", "l", false);
      result = true;
      break;
    case a::justify::right:
      v("labeljust", "r", false);
      result = true;
      break;
    case a::justify::center:
      v("labeljust", "c", false);
      break;
    }

    switch (splines) {
    case a::spline_type::no_value:
      break;
    case a::spline_type::none:
      v("splines", "none", false);
      result = true;
      break;
    case a::spline_type::line:
      v("splines", "line", false);
      result = true;
      break;
    case a::spline_type::polyline:
      v("splines", "polyline", false);
      result = true;
      break;
    case a::spline_type::curved:
      v("splines", "curved", false);
      result = true;
      break;
    case a::spline_type::ortho:
      v("splines", "ortho", false);
      result = true;
      break;
    case a::spline_type::spline:
      v("splines", "spline", false);
      result = true;
      break;
    }
    result |= visit_style_attribute(v, style);
    if (foreground_color_list) {
      v("color", foreground_color_list, true);
      result = true;
    } else {
      result |= visit_color_attribute(v, "color", foreground_color);
    }
    result |= visit_color_attribute(v, "bgcolor", background_color);
    return result;
  }
}
