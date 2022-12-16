#pragma once

#include "containers/pair.hpp"
#include "containers/trees/rope_tree_concepts.hpp"
#include "containers/trees/rope_tree_iterator.hpp"

namespace j::inline containers::trees {
  /// Reference to the containing tree node, stored in leaf nodes.
  ///
  /// Lead node must store rope_tree_ref in a public field named `ref`.
  template<RopeTreeDescription DescriptionT>
  class rope_tree_ref final {
  public:
    using node_t = rope_tree_node<DescriptionT>;
    using value_t = typename DescriptionT::value_t;
    using iterator_t = rope_tree_iterator<node_t, false>;
    using key_t = typename DescriptionT::key_t;
    using key_diff_t = typename DescriptionT::key_diff_t;
  private:
    uptr_t m_node_and_index = 0U;
    u32_t m_modification_stamp = 0U;
    friend node_t;

    void set(node_t * J_NOT_NULL node, u8_t index) noexcept {
      J_ASSUME(index < DescriptionT::max_size_v);
      m_node_and_index = ((uptr_t)node << 8) | (uptr_t)index;
    }
  public:
    J_ALWAYS_INLINE void set_modification_stamp(u32_t stamp) noexcept
    { m_modification_stamp = stamp; }

    J_INLINE_GETTER u32_t modification_stamp() const noexcept
    { return m_modification_stamp; }

    J_INLINE_GETTER u8_t index() const {
      u8_t idx = m_node_and_index;
      J_ASSERT(idx < DescriptionT::max_size_v);
      return idx;
    }

    J_INLINE_GETTER_NONNULL node_t * node() noexcept {
      return reinterpret_cast<node_t*>(m_node_and_index >> 8);
      // J_ASSERT(!result || (result->is_leaf() && ::j::max(1, result->size()) > index()), "Invalid reference");
      // return result;
    }

    /// Get the containing node of the value.
    J_INLINE_GETTER_NONNULL const node_t * node() const noexcept
    { return const_cast<rope_tree_ref *>(this)->node(); }

    J_INLINE_GETTER iterator_t as_iterator() noexcept
    { return iterator_t{node(), (i8_t)index()}; }

    /// Get the next value, or null if first.
    [[nodiscard]] typename DescriptionT::value_t * next() noexcept {
      const u8_t idx = index();
      const auto n = node();
      if (idx < n->size() - 1U) {
        return &n->value_at(idx + 1U);
      }
      return n->next ? &n->next->value_at(0U) : nullptr;
    }

    /// Get the next value, or null if first.
    J_INLINE_GETTER const typename DescriptionT::value_t * next() const noexcept
    { return const_cast<rope_tree_ref*>(this)->next(); }

    /// Get the previous value, or null if first.
    [[nodiscard]] typename DescriptionT::value_t * previous() noexcept {
      const u8_t idx = index();
      const auto n = node();
      if (idx == 0U) {
        const auto prev = n->previous;
        if (!prev) {
          return nullptr;
        }
        const u8_t sz = n->previous->size();
        J_ASSUME(sz >= DescriptionT::max_size_v / 2U);
        return &prev->value_at(sz - 1U);
      }
      return &n->value_at(idx - 1U);
    }

    /// Get the previous value, or null if first.
    J_INLINE_GETTER const typename DescriptionT::value_t * previous() const noexcept
    { return const_cast<rope_tree_ref*>(this)->previous(); }

    /// Notify that the metrics of the node have changed by delta.
    J_ALWAYS_INLINE void propagate_state_change(
      u32_t modification_stamp,
      typename DescriptionT::state_diff_t & delta) noexcept
    {
      node()->propagate_state_change(modification_stamp, index(), delta);
    }
  };
}
