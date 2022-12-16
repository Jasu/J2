#pragma once

#include "graphviz/common.hpp"
#include "attributes/attribute_definition.hpp"
#include "colors/rgb.hpp"
#include "strings/string_view.hpp"

namespace j::graphviz::attributes {
  /// Node, edge, graph, or subgraph style.
  enum class style : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    filled,
    striped,
    solid,
    dashed,
    dotted,
    bold,
    invis,
  };

  enum class justify : u8_t {
    no_value,

    left,
    center,
    right,
  };

  enum class align : u8_t {
    no_value,

    top,
    center,
    bottom,
  };

  namespace a = j::attributes;
  /// Text color. Common to all object types.
  constexpr inline const a::attribute_definition font_color{
    a::value_type = type<color>,
    a::tag = type<struct font_color_tag>};

  /// Font size in points. Common to all object types.
  constexpr inline const a::attribute_definition font_size{
    a::value_type = type<float>,
    a::tag = type<struct font_size_tag>};

  /// Font family name. Common to all object types.
  constexpr inline const a::attribute_definition font_name{
    a::value_type = type<strings::const_string_view>,
    a::tag = type<struct font_name_tag>};

  /// CSS class name. Common to all object types.
  constexpr inline const a::attribute_definition css_class{
    a::value_type = type<strings::const_string_view>,
    a::tag = type<struct css_class_tag>};

  /// ID of a node.
  constexpr inline const a::attribute_definition id{
    a::value_type = type<strings::const_string_view>,
    a::tag = type<struct id_tag>};

  /// Label (to be displayed instead of the name). Common to all object types.
  constexpr inline const a::attribute_definition label{
    a::value_type = type<strings::const_string_view>,
    a::tag = type<struct label_tag>};

  /// If set, label is considered to be HTML.
  constexpr inline const a::attribute_definition label_is_html{
    a::value_type = type<bool>,
    a::tag = type<struct label_is_html_tag>,
    a::is_flag};

  /// Tooltip text. Specific to nodes and edges.
  constexpr inline const a::attribute_definition tooltip{
    a::value_type = type<strings::const_string_view>,
    a::tag = type<struct tooltip_tag>};

  /// Edge style. Specific to edges.
  constexpr inline const a::attribute_definition edge_style{
    a::value_type = type<style>,
    a::tag = type<style>};

  /// Arrow size. Specific to edges.
  constexpr inline const a::attribute_definition arrow_size{
    a::value_type = type<float>,
    a::tag = type<struct arrow_size>};

  enum class clip : u8_t {
    both,
    head,
    tail,
    none,
  };

  /// Whether to clip arrow head or tail to target edge. Default true.
  constexpr inline const a::attribute_definition clip_type{a::value_type = type<clip>, a::tag = type<clip>};

  /// For nodes, the min width in inches. For graphs, the viewport width in inches. For edges, penwidth.
  constexpr inline const a::attribute_definition width{
    a::value_type = type<float>,
    a::tag = type<struct width_tag>};

  /// For nodes, the minimum height in inches. For graphs, the viewport height in inches.
  ///
  /// Specific to nodes and graphs.
  constexpr inline const a::attribute_definition height{
    a::value_type = type<float>,
    a::tag = type<struct height_tag>};

  /// Fill style. Specific to nodes.
  constexpr inline const a::attribute_definition node_style{
    a::value_type = type<style>,
    a::tag = type<struct node_style_tag>};

  /// Predefined node shapes.
  enum class node_shape : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    plain,
    plaintext,
    ellipse,
    oval,
    circle,
    none,
    doublecircle,
    egg,
    triangle,
    box,
    diamond,
    trapezium,
    parallelogram,
    house,
    hexagon,
    octagon,
    note,
    tab,
    box3d,
    component,
  };


  /// Node shape. Specific to nodes.
  constexpr inline const a::attribute_definition shape{
    a::value_type = type<node_shape>,
    a::tag = type<struct shape_tag>};

  /// Outline color. Specific to nodes.
  constexpr inline const a::attribute_definition outline_color{
    a::value_type = type<color>,
    a::tag = type<struct outline_color_tag>};

  /// Fill (background) color. Specific to nodes.
  constexpr inline const a::attribute_definition fill_color{
    a::value_type = type<color>,
    a::tag = type<struct fill_color_tag>};




  /// Arrow direction for the edge.
  enum class direction : u8_t{
    /// Default value, resulting in no Dot output.
    no_value,

    forward,
    backward,
    both,
    none,
  };

  /// Arrow style for edges.
  enum class arrow : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    none,
    normal,
    inv,
    dot,
    odot,
    invdot,
    invodot,
    tee,
    empty,
    invempty,
    open,
    halfopen,
    diamond,
    odiamond,
    box,
    obox,
    crow,
  };

  /// Edge color. Specific to edges.
  constexpr inline const a::attribute_definition edge_color{
    a::value_type = type<color>,
    a::tag = type<struct edge_color_tag>};

  /// Edge arrow direction. Specific to edges.
  constexpr inline const a::attribute_definition arrow_direction{
    a::value_type = type<direction>,
    a::tag = type<direction>};

  /// Style of the arrow head. Specific to edges.
  constexpr inline const a::attribute_definition arrow_head{
    a::value_type = type<arrow>,
    a::tag = type<arrow>};

  /// Style of the arrow tail. Specific to edges.
  constexpr inline const a::attribute_definition arrow_tail{
    a::value_type = type<arrow>,
    a::tag = type<struct arrow_tail_tag>};

  enum class rank_dir : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    top_to_bottom,
    left_to_right,
    bottom_to_top,
    right_to_left,
  };

  enum class spline_type : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    none,
    line,
    polyline,
    curved,
    ortho,
    spline,
  };

  /// Values for ordering. Specific to graphs.
  enum class order: u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    out,
    in,
  };

  /// Values for outputMode. Specific to root graphs.
  enum class output_mode : u8_t {
    /// Default value, resulting in no Dot output.
    no_value,

    breadth_first,
    nodes_first,
    edges_first,
  };

  /// How edges are drawn. Specific to graphs.
  constexpr inline const a::attribute_definition splines{
    a::value_type = type<spline_type>,
    a::tag = type<spline_type>};

  constexpr inline const a::attribute_definition new_rank{
    a::is_flag, a::tag = type<struct new_rank_tag>};

  constexpr inline const a::attribute_definition output_order{
    a::value_type = type<output_mode>,
    a::tag = type<output_mode>};

  constexpr inline const a::attribute_definition ordering{
    a::value_type = type<order>,
    a::tag = type<order>};

  /// For graphs, page margin in inches. For nodes, margin around label.
  constexpr inline const a::attribute_definition margin{
    a::value_type = type<float>,
    a::tag = type<struct margin_tag>};

  /// Minimum distance between ranks in graphs.
  constexpr inline const a::attribute_definition ranksep{
    a::value_type = type<float>,
    a::tag = type<struct ranksep_tag>};

  /// Equal spacing between ranks
  constexpr inline const a::attribute_definition ranksep_equally{
    a::is_flag,
    a::tag = type<struct ranksep_equally_tag>};

  /// Minimum distance between nodes in the same rank in graphs.
  constexpr inline const a::attribute_definition nodesep{
    a::value_type = type<float>,
    a::tag = type<struct nodesep_tag>};

  /// Foreground color, used by graph style.
  constexpr inline const a::attribute_definition foreground_color{
    a::value_type = type<color>,
    a::tag = type<struct foreground_color_tag>};

  /// Background color, used by graph style.
  constexpr inline const a::attribute_definition background_color{
    a::value_type = type<color>,
    a::tag = type<struct background_color_tag>};

  /// If set on a graph, edges sharing endpoints get merged.
  constexpr inline const a::attribute_definition concentrate{
    a::is_flag,
    a::tag = type<struct concentrate_tag>};

  enum class rank_type : u8_t {
    no_value,
    same,
    min,
    max,
    source,
    sink,
  };

  /// If set, nodes in the subgraphs are placed along a same line.
  ///
  /// Specific to graphs and subgraphs.
  constexpr inline const a::attribute_definition rank{
    a::value_type = type<rank_type>,
    a::tag = type<rank_type>};

  constexpr inline const a::attribute_definition rank_direction{
    a::value_type = type<rank_dir>,
    a::tag = type<rank_dir>};

  /// Graph style. Specific to graphs and subgraphs.
  constexpr inline const a::attribute_definition graph_style{
    a::value_type = type<style>,
    a::tag = type<style>};

  constexpr inline const a::attribute_definition label_justify{
    a::value_type = type<justify>,
    a::tag = type<justify>};

  constexpr inline const a::attribute_definition label_align{
    a::value_type = type<align>,
    a::tag = type<align>};
}
