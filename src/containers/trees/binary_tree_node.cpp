#include "containers/trees/binary_tree_node.hpp"

namespace j::inline containers::trees {
  u32_t binary_tree_node_base::depth() const noexcept {
    return 1U + j::max(left ? left->depth() : 0U,
                       right ? right->depth() : 0U);
  }

  void binary_tree_node_base::destroy() noexcept {
    if (left) {
      left->destroy();
    }
    if (right) {
      right->destroy();
    }
    ::delete this;
  }
}
