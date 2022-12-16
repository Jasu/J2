#pragma once

#include "hzd/iterators.hpp"

#include "graphviz/graph_attributes.hpp"
#include "graphviz/node_attributes.hpp"
#include "graphviz/edge_attributes.hpp"
#include "graphviz/edge.hpp"
#include "graphviz/node.hpp"

namespace j::graphviz {
  struct graph;
}

namespace j::graphviz::attributes {
  /// If set, the subgraph will be a clustered graph. Specific to subgraphs.
  inline constexpr a::attribute_definition is_cluster{
    a::is_flag,
    a::tag = type<struct is_cluster_tag>};

  /// Default node attributes for the graph.
  constexpr inline const a::attribute_definition default_node_attributes(
    a::value_type = type<node_attributes>,
    a::tag = type<node_attributes>);

  /// Default edge attributes for the graph.
  constexpr inline const a::attribute_definition default_edge_attributes(
    a::value_type = type<edge_attributes>,
    a::tag = type<edge_attributes>);
  /// If set, the graph will be directional. Specific to graphs.
  inline constexpr a::attribute_definition is_directional{
    a::is_flag, a::tag = type<struct is_directional_tag>};
}

namespace j::graphviz {
  namespace a = j::graphviz::attributes;

  enum class graph_child_type : u8_t {
    node,
    graph,
    secondary_node,
  };

  struct graph_child final {
    graph_child_type type;
    union {
      graph * graph = nullptr;
      node * node;
      void * ptr;
    };
    graph_child * next = nullptr;
  };

  struct graph final {
    graph() noexcept;
    ~graph();

    template<typename... Args>
    J_A(ND,AI,HIDDEN) inline explicit graph(Args && ... args)
      : attributes(static_cast<Args &&>(args)...),
        is_cluster(j::attributes::has<Args...>(a::is_cluster)),
        is_directional(j::attributes::has<Args...>(a::is_directional)),
        is_new_rank(j::attributes::has<Args...>(a::new_rank))
    {
      namespace aa = ::j::attributes;
      if constexpr (aa::has<Args...>(a::default_node_attributes)) {
        default_node_attributes = a::default_node_attributes.get(static_cast<Args &&>(args)...);
      }
      if constexpr (aa::has<Args...>(a::default_edge_attributes)) {
        default_edge_attributes = a::default_edge_attributes.get(static_cast<Args &&>(args)...);
      }
    }

    void add_child(graph_child_type type, void * J_NOT_NULL child) noexcept;

    template<typename... Args>
    [[nodiscard]] J_NO_DEBUG J_RETURNS_NONNULL graph * add_subgraph(Args && ... args) {
      graph * n = ::new graph(static_cast<Args &&>(args)...);
      add_child(graph_child_type::graph, n);
      return n;
    }

    void add_secondary_node(node * J_NOT_NULL n);

    template<typename... Args>
    J_NO_DEBUG J_ALWAYS_INLINE_NONNULL node * add_node(Args && ... args) {
      node * n = ::new node(static_cast<Args &&>(args)...);
      add_child(graph_child_type::node, n);
      return n;
    }

    template<typename... Args>
    J_NO_DEBUG J_RETURNS_NONNULL inline edge * add_edge(Args && ... args) {
      auto e = ::new edge(static_cast<Args &&>(args)...);
      if (last_edge) {
        last_edge->next = e;
      } else {
        first_edge = e;
      }
      last_edge = e;
      return e;
    }

    struct graph_child_view children() const noexcept;
    struct edge_view edges() const noexcept;

    graph_child * first_child = nullptr;
    graph_child * last_child = nullptr;

    edge * first_edge = nullptr;
    edge * last_edge = nullptr;

    graph_attributes attributes;
    node_attributes default_node_attributes;
    edge_attributes default_edge_attributes;
    bool is_cluster:1 = false;
    bool is_directional:1 = false;
    bool is_new_rank:1 = false;
  };

  struct graph_child_view final {
    const graph * g;

    J_INLINE_GETTER linked_list_iterator<graph_child> begin() const noexcept {
      return {g->first_child};
    }

    J_INLINE_GETTER linked_list_iterator<graph_child> end() const noexcept {
      return {nullptr};
    }
  };

  struct edge_view final {
    const graph * g;

    J_INLINE_GETTER linked_list_iterator<edge> begin() const noexcept {
      return {g->first_edge};
    }

    J_INLINE_GETTER linked_list_iterator<edge> end() const noexcept {
      return {nullptr};
    }
  };
}
