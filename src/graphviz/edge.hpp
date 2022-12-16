#pragma once

#include "graphviz/edge_attributes.hpp"

namespace j::graphviz {
  struct node;
  struct graph;

  struct endpoint final {
    node * node = nullptr;
    graph * subgraph = nullptr;
    strings::string port;

    J_BOILERPLATE(endpoint, CTOR_NE_ND)

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return node;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !node;
    }

    endpoint(struct node * J_NOT_NULL node, graph * subgraph, strings::string && port) noexcept;

    J_A(AI,ND) inline endpoint(struct node * J_NOT_NULL node, graph * subgraph = nullptr) noexcept
      : node(node),
        subgraph(subgraph)
    { }

    J_A(AI,ND) inline endpoint(struct node * J_NOT_NULL node, strings::string && port) noexcept
      : node(node),
        port(static_cast<strings::string &&>(port))
    { }

    [[nodiscard]] endpoint with_subgraph(graph * subgraph J_NOT_NULL) noexcept {
      return {node, subgraph, strings::string(port)};
    }
  };

  struct edge final {
    edge() noexcept;

    template<typename From, typename To>
    J_A(AI,ND,HIDDEN) inline edge(From && from, To && to) noexcept
      : from(static_cast<From &&>(from)),
        to(static_cast<To &&>(to))
    {
    }

    template<typename From, typename To, typename... Args>
    J_A(AI,ND,HIDDEN) inline edge(From && from, To && to, Args && ...args)
      : from(static_cast<From &&>(from)),
        to(static_cast<To &&>(to)),
        attributes(static_cast<Args &&>(args)...)
    {
    }

    endpoint from;
    endpoint to;
    edge_attributes attributes;
    edge * next = nullptr;
  };
}
