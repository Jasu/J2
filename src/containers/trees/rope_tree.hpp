#pragma once

#include "containers/trees/rope_tree_fwd.hpp"
#include "containers/trees/rope_tree_node.hpp"

namespace j::inline containers::trees {
  template<RopeTreeDescription Desc>
  u32_t rope_tree<Desc>::modification_stamp_between(const_iterator first, const_iterator last) noexcept {
    if (!first) { return modification_stamp; }
    u32_t value = first->ref.modification_stamp();
    while (first++ != last) {
      value = ::j::max(value, first->ref.modification_stamp());
    }
    return value;
  }

  template<RopeTreeDescription Desc>
  pair<typename rope_tree<Desc>::iterator, typename rope_tree<Desc>::key_t>
  rope_tree<Desc>::find(key_t key)
  { return root.find(key); }

  template<RopeTreeDescription Desc>
  typename rope_tree<Desc>::iterator rope_tree<Desc>::split(key_t key) {
    const auto p = find(key);
    return split(p.first, p.second);
  }

  template<RopeTreeDescription Desc>
  typename rope_tree<Desc>::iterator rope_tree<Desc>::split(const_iterator it, key_t offset) {
    J_ASSUME(offset != 0);
    const iterator & it_ = reinterpret_cast<const iterator &>(it);
    return it_.node->split_at(it_.index, offset);
  }

  template<RopeTreeDescription Desc>
  typename rope_tree<Desc>::iterator rope_tree<Desc>::erase(
    const_iterator it,
    key_t offset,
    key_t sz
  ) {
    iterator it_ = reinterpret_cast<const iterator &>(it);
    J_ASSERT_NOT_NULL(it_, sz);
    key_diff_t node_sz = it_.key_delta();
    J_ASSERT((key_diff_t)offset < node_sz, "Offset out of range.");
    const key_diff_t suffix_sz = node_sz - offset;

    // Special case for deleting a portion from the middle.
    if (offset && suffix_sz > (key_diff_t)sz) {
      it_ = it_.node->erase_middle(++modification_stamp, it_, offset, sz);
      J_ASSERT(it_.key_delta() == suffix_sz - (key_diff_t)sz, "Erase end mismatch.");
      J_ASSERT(it_.previous().key_delta() == (key_diff_t) offset, "Erase begin mismatch.");
      return it_;
    }

    state_diff_t chg(it_.initialize_diff());
    if (offset) {
      J_ASSERT(suffix_sz <= (key_diff_t)sz, "Out of range.");
      J_ASSERT(it_.key_delta() == suffix_sz + (key_diff_t)offset, "Out of range");
      it_->erase_suffix(chg, suffix_sz);
      J_ASSERT(chg.metrics_delta().key() == -suffix_sz, "Change mismatch.");
      it_.node->notify_metrics_delta(it_, chg);
      J_ASSERT(it_.key_delta() == (key_diff_t)offset, "Size after mismatch");
      ++it_;
      sz -= suffix_sz;
    }

    while (sz) {
      J_ASSERT_NOT_NULL(it_);
      node_sz = it_.key_delta();
      if ((key_diff_t)sz >= node_sz) {
        it_ = it_.node->erase(chg, it_);
        sz -= node_sz;
      } else {
        it_->erase_prefix(chg, sz);
        J_ASSERT(chg.metrics_delta().key() == -(key_diff_t)sz, "Change mismatch");
        it_.node->notify_metrics_delta(it_, chg);
        J_ASSERT(it_.key_delta() == node_sz - (key_diff_t)sz, "Size after mismatch");
        break;
      }
    }

    if (it_) {
      it_.node->notify_update_state(++modification_stamp, it_, chg);
    }

    return it_;
  }

  template<RopeTreeDescription Desc>
  typename rope_tree<Desc>::iterator rope_tree<Desc>::erase(key_t start, key_t sz) {
    const auto p = find(start);
    return erase(p.first, p.second, sz);
  }

  template<RopeTreeDescription Desc>
  typename rope_tree<Desc>::iterator rope_tree<Desc>::erase(const_iterator it) {
    auto diff = it.initialize_diff();
    auto it2 = const_cast<node_t*>(it.node)->erase(diff, it);
    if (it2) {
      it2.node->notify_update_state(++modification_stamp, it2, diff);
    }
    return it2;
  }
}

#define J_DEFINE_EXTERN_ROPE_TREE(DESC)                      \
  template struct j::containers::trees::rope_tree_iterator<  \
    j::containers::trees::rope_tree_node<DESC>, true>;       \
  template struct j::containers::trees::rope_tree_iterator<  \
    j::containers::trees::rope_tree_node<DESC>, false>;      \
  template class j::containers::trees::rope_tree_node<DESC>; \
  template class j::containers::trees::rope_tree<DESC>
