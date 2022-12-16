#include "graphviz/edge_attributes.hpp"
#include "strings/string.hpp"
#include "graphviz/edge.hpp"
#include "strings/format.hpp"

namespace j::graphviz {
  namespace {
    bool visit_arrow_attribute(attribute_visitor_t & v, const char * key, a::arrow value) {
      switch (value) {
      case a::arrow::no_value:
        return false;
      case a::arrow::none:
        v(key, "none", false);
        return true;
      case a::arrow::normal:
        v(key, "normal", false);
        return true;
      case a::arrow::inv:
        v(key, "inv", false);
        return true;
      case a::arrow::dot:
        v(key, "dot", false);
        return true;
      case a::arrow::odot:
        v(key, "odot", false);
        return true;
      case a::arrow::invdot:
        v(key, "invdot", false);
        return true;
      case a::arrow::invodot:
        v(key, "invodot", false);
        return true;
      case a::arrow::tee:
        v(key, "tee", false);
        return true;
      case a::arrow::empty:
        v(key, "empty", false);
        return true;
      case a::arrow::invempty:
        v(key, "invempty", false);
        return true;
      case a::arrow::open:
        v(key, "open", false);
        return true;
      case a::arrow::halfopen:
        v(key, "halfopen", false);
        return true;
      case a::arrow::diamond:
        v(key, "diamond", false);
        return true;
      case a::arrow::odiamond:
        v(key, "odiamond", false);
        return true;
      case a::arrow::box:
        v(key, "box", false);
        return true;
      case a::arrow::obox:
        v(key, "obox", false);
        return true;
      case a::arrow::crow:
        v(key, "crow", false);
        return true;
      }
    }
  }

  edge_attributes::edge_attributes() noexcept {
  }

  bool edge_attributes::visit_attributes(attribute_visitor_t && v) const {
    bool result = common_attributes::visit_attributes(static_cast<attribute_visitor_t &&>(v));
    result |= visit_style_attribute(v, style);
    result |= visit_int_attribute(v, "minlen", minlen, true);
    result |= visit_float_attribute(v, "penwidth", pen_width, true);
    result |= visit_float_attribute(v, "arrowsize", arrow_size, true);
    result |= visit_color_attribute(v, "color", edge_color);
    if (!constraint) {
      result = true;
      v("constraint", "false", false);
    }

    switch (arrow_direction) {
    case a::direction::no_value:
      break;
    case a::direction::forward:
      v("dir", "forward", false);
      result = true;
      break;
    case a::direction::backward:
      v("dir", "back", false);
      result = true;
      break;
    case a::direction::both:
      v("dir", "both", false);
      result = true;
      break;
    case a::direction::none:
      v("dir", "none", false);
      result = true;
      break;
    }

    if (clip_type == a::clip::none || clip_type == a::clip::tail) {
      result = true;
      v("headclip", "false", false);
    }
    if (clip_type == a::clip::none || clip_type == a::clip::head) {
      result = true;
      v("tailclip", "false", false);
    }

    result |= visit_arrow_attribute(v, "arrowhead", arrow_head);
    result |= visit_arrow_attribute(v, "arrowtail", arrow_tail);
    if (weight >= 0) {
      v("weight", strings::format("{}", weight), false);
      result = true;
    }
    return result;
  }
  edge::edge() noexcept { }

  endpoint::endpoint(struct node * J_NOT_NULL node, graph * subgraph, strings::string && port) noexcept
    : node(node),
      subgraph(subgraph),
      port(static_cast<strings::string &&>(port))
  { }
}
