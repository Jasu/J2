#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::inline containers::trees {
  template<typename NodeT, bool IsConst>
  struct rope_tree_iterator final {
    using node_t = const_if_t<IsConst, NodeT>;
    using key_t = typename NodeT::key_t;
    using key_diff_t = typename NodeT::key_diff_t;
    using state_diff_t = typename NodeT::state_diff_t;
    using value_t = const_if_t<IsConst, typename NodeT::value_t>;
    using metrics_t = typename NodeT::metrics_t;

    node_t * node = nullptr;
    i8_t index = 0U;

    rope_tree_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(node);
      J_ASSERT((u8_t)index < node->size(), "Increment out of bounds");
      if (node->size() == ++index && node->next) {
        node = node->next;
        index = 0U;
      }
      return *this;
    }

    J_INLINE_GETTER bool has_next() const noexcept {
      J_ASSUME_NOT_NULL(node);
      return index + 1 < node->size() || node->next;
    }

    J_INLINE_GETTER bool has_previous() const noexcept {
      J_ASSUME_NOT_NULL(node);
      return index > 0 || node->previous;
    }

    rope_tree_iterator next() const noexcept {
      J_ASSUME(node != nullptr);
      J_ASSERT((u8_t)index < node->size(), "Increment out of bounds");
      if (node->size() == index + 1 && node->next) {
        return { node->next, (i8_t)0 };
      }
      return { node, (i8_t)(index + 1) };
    }

    J_INLINE_GETTER metrics_t metrics_delta() const noexcept
    { return node->metrics_delta(index); }

    J_INLINE_GETTER key_t position() const noexcept
    { return node->position(index); }

    J_INLINE_GETTER key_diff_t key_delta() const noexcept
    { return node->key_delta(index); }

    J_INLINE_GETTER metrics_t cumulative_metrics() const noexcept
    { return node->cumulative_metrics(index); }

    rope_tree_iterator & operator--() noexcept {
      J_ASSUME_NOT_NULL(node);
      J_ASSUME(index >= 0);
      if (index-- == 0 && node->previous) {
        node = node->previous;
        index = node->size() - 1U;
      }
      return *this;
    }

    rope_tree_iterator previous() const noexcept {
      J_ASSUME(node != nullptr);
      J_ASSUME(index >= 0);
      if (index == 0 && node->previous) {
        return { node->previous, (i8_t)(node->previous->size() - 1) };
      }
      return { node, (i8_t)(index - 1) };
    }

    state_diff_t initialize_diff() const noexcept {
      if (index) {
        return state_diff_t(&node->value_at(index - 1));
      }
      const auto prev = node->previous;
      return state_diff_t(prev ? &prev->value_at(prev->size() - 1) : nullptr);
    }

    /// The iterator points to a very specific nowhere, i.e. a dummy node in an empty rope.
    J_INLINE_GETTER bool is_at_a_nowhere() const noexcept {
      return index == 0 && node->size() == 0;
    }

    /// The iterator is at the end of not nothing since nothing by definition does not exist.
    ///
    /// \note Maybe rename this function - the need for it arises from representing an
    ///       empty rope as a single empty node with the size of the rope marked as zero.
    ///       To avoid checks for NULL, I made NULL into a concrete object, causing an
    ///       entirely different set of problems, and blamed its physical existence on Sartre.
    ///
    ///       > All human activities are equivalent... and all are on principle doomed
    ///       >to failure.
    ///         - Jean-Paul Sartre
    J_INLINE_GETTER bool is_at_end_fuck_sartre() const noexcept
    { return index && node->size() == index; }

    J_INLINE_GETTER bool is_at_end() const noexcept
    { return node->size() == index; }

    J_INLINE_GETTER bool is_at_begin() const noexcept
    { return index == 0 && !node->previous; }

    J_INLINE_GETTER bool is_before_begin() const noexcept
    { return index < 0; }

    [[nodiscard]] J_RETURNS_NONNULL value_t * operator->() const noexcept
    { return &node->value_at(index); }

    J_INLINE_GETTER value_t & operator*() const noexcept
    { return *operator->(); }

    J_ALWAYS_INLINE rope_tree_iterator operator++(int) noexcept {
      const auto result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE rope_tree_iterator operator--(int) noexcept {
      const auto result{*this};
      return operator--(), result;
    }

    J_INLINE_GETTER operator const rope_tree_iterator<NodeT, true> &() const noexcept
    { return *reinterpret_cast<const rope_tree_iterator<NodeT, true> *>(this); }

    J_INLINE_GETTER bool operator==(const rope_tree_iterator & rhs) const noexcept
    { return node == rhs.node && index == rhs.index; }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return index >= 0 && index < node->size(); }

    J_INLINE_GETTER bool operator!() const noexcept
    { return index < 0 || index >= node->size(); }
  };
}
