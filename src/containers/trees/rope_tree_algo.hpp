#pragma once

#include "containers/trees/rope_tree_node_fwd.hpp"

namespace j::inline containers::trees {
  template<typename Desc>
  bool rope_tree_node_before(
    const rope_tree_node<Desc> * J_NOT_NULL lhs_node,
    const rope_tree_node<Desc> * J_NOT_NULL rhs_node) noexcept
  {
    u8_t lhs_index = 0U, rhs_index = 0U;
    while (lhs_node != rhs_node) {
      lhs_index = lhs_node->index();
      rhs_index = rhs_node->index();
      lhs_node = lhs_node->parent();
      rhs_node = rhs_node->parent();
    }
    return lhs_index < rhs_index;
  }

  template<typename ValueT>
  bool rope_value_before(const ValueT & lhs_value, const ValueT & rhs_value) {
    const auto * const lhs_node = lhs_value.ref.node();
    const auto * const rhs_node = rhs_value.ref.node();
    if (lhs_node == rhs_node) {
      return lhs_value.ref.index() < rhs_value.ref.index();
    }
    return rope_tree_node_before(lhs_node, rhs_node);
  }

  template<typename Desc, typename Delta = typename Desc::key_diff_t>
  Delta rope_tree_node_distance(
    const rope_tree_node<Desc> * J_NOT_NULL lhs_node,
    const rope_tree_node<Desc> * J_NOT_NULL rhs_node) noexcept
  {
    Delta result = 0;
    while (lhs_node != rhs_node) {
      const u8_t lhs_index = lhs_node->index(), rhs_index = rhs_node->index();
      lhs_node = lhs_node->parent();
      rhs_node = rhs_node->parent();
      if (lhs_index) {
        result -= lhs_node->metrics_at(lhs_index - 1U).key();
      }
      if (rhs_index) {
        result += rhs_node->metrics_at(rhs_index - 1U).key();
      }
    }
    return result;
  }

  template<typename ValueT>
  auto rope_value_distance(const ValueT & lhs_value, const ValueT & rhs_value) {
    const auto * const lhs_node = lhs_value.ref.node();
    const auto * const rhs_node = rhs_value.ref.node();
    const u8_t lhs_index = lhs_value.ref.index();
    const u8_t rhs_index = rhs_value.ref.index();
    return (rhs_index ? rhs_node->metrics_at(rhs_index - 1U).key() : 0)
         - (lhs_index ? lhs_node->metrics_at(lhs_index - 1U).key() : 0)
         + rope_tree_node_distance(lhs_node, rhs_node);
  }
}
