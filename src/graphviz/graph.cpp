#include "graphviz/graph.hpp"

namespace j::graphviz {
  graph::graph() noexcept { }

  graph::~graph() {
    for (edge * e = first_edge; e;) {
      auto * const next = e->next;
      ::delete e;
      e = next;
    }

    for (graph_child * ch = first_child; ch;) {
      graph_child * const next = ch->next;
      switch (ch->type) {
      case graph_child_type::node:
        ::delete ch->node;
        break;
      case graph_child_type::graph:
        ::delete ch->graph;
        break;
      case graph_child_type::secondary_node:
        break;
      }
      ::delete ch;
      ch = next;
    }
  }

  void graph::add_child(graph_child_type type, void * J_NOT_NULL child) noexcept {
    auto ch = ::new graph_child{
      type,
      { .ptr = child },
      nullptr,
    };
    if (last_child) {
      last_child->next = ch;
    } else {
      first_child = ch;
    }
    last_child = ch;
  }

  void graph::add_secondary_node(node * J_NOT_NULL n) {
    add_child(graph_child_type::secondary_node, n);
  }

  graph_child_view graph::children() const noexcept {
    return { this };
  }

  edge_view graph::edges() const noexcept {
    return { this };
  }
}
