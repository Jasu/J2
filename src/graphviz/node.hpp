#pragma once

#include "graphviz/node_attributes.hpp"

namespace j::graphviz {
  namespace a = j::graphviz::attributes;

  struct node final {
    node_attributes attributes;

    node() noexcept;

    template<typename... Args>
    J_A(AI,ND) inline explicit node(Args && ... args)
      : attributes(static_cast<Args &&>(args)...)
    {
    }
  };
}
