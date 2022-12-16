#pragma once

#include "hzd/string.hpp"
#include "containers/trees/rope_tree_node_fwd.hpp"

namespace j::inline containers::trees {
  template<RopeTreeDescription Desc>
  rope_tree_node<Desc>::rope_tree_node(rope_tree_node && rhs) noexcept
  { copy_from(rhs); }

  template<RopeTreeDescription Desc>
  rope_tree_node<Desc> & rope_tree_node<Desc>::operator=(rope_tree_node && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      destroy();
      copy_from(rhs);
    }
    return *this;
  }

  template<RopeTreeDescription Desc>
  rope_tree_node<Desc>::rope_tree_node(
    rope_tree_node * J_NOT_NULL J_RESTRICT left_child,
    rope_tree_node * J_NOT_NULL J_RESTRICT right_child
  ) noexcept
    : m_parent_and_data(2U | is_inner_mask_v)
  {
    m_data[0].set_node_ptr(left_child);
    m_data[1].set_node_ptr(right_child);
    m_metrics[0] = left_child->metrics();
    m_metrics[1] = m_metrics[0] + right_child->metrics();
    right_child->next = left_child->previous = nullptr;
    left_child->next = right_child;
    right_child->previous = left_child;
    left_child->set_parent_and_index(this, 0);
    right_child->set_parent_and_index(this, 1);
  }

  template<RopeTreeDescription Desc>
  pair<typename rope_tree_node<Desc>::iterator, typename Desc::key_t>
  rope_tree_node<Desc>::find(const key_t key) {
    const u8_t sz = size();
    key_t previous_key = 0;
    for (u8_t i = 0U; i < sz; ++i) {
      const key_t metrics_key = m_metrics[i].key();
      if (metrics_key > key) {
        return is_inner()
          ? node_at(i).find(key - previous_key)
          : pair<iterator, key_t>{{ this, (i8_t)i}, key - previous_key};
      }
      previous_key = metrics_key;
    }
    J_REQUIRE(previous_key == key, "Out of range");
    return { end(), 0 };
  }

  template<RopeTreeDescription Desc>
  typename rope_tree_node<Desc>::iterator rope_tree_node<Desc>::split_at(u8_t index, key_t offset) {
    const u8_t sz = size();
    state_diff_t diff(previous_diff(index));
    auto & val = value_at(index);
    auto result = val.split(diff, offset);
    result.ref.set_modification_stamp(val.ref.modification_stamp());
    auto prev_metrics = m_metrics[index];
    auto metrics_delta = diff.metrics_delta();
    m_metrics[index] += metrics_delta;
    if (sz != max_size_v) {
      emplace_value_at_no_metrics(index + 1U, static_cast<value_t &&>(result));
      m_metrics[index + 1U] = prev_metrics;
      return { this, (i8_t)index };
    }
    for (u8_t i = index + 1U; i < sz; ++i) {
      m_metrics[i] += metrics_delta;
    }
    split_result split_res = split(index + 1U, { this, (i8_t)index });
    split_res.target_node->emplace_value_at_known_metrics(
      split_res.target_index,
      -metrics_delta,
      static_cast<value_t &&>(result));
    if (rope_tree_node * const p = parent()) {
      return p->handle_split_upwards(split_res, this->index()).tracked_iterator;
    } else {
      return handle_split_root(split_res).tracked_iterator;
    }
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::notify_metrics_delta(iterator it, state_diff_t & diff) {
    J_ASSERT(is_leaf() && it.node == this && it.index < size(), "Iterator mismatch");
    auto metrics_delta = diff.metrics_delta();
    if (metrics_delta) {
      diff.reset_metrics_delta();
      apply_metrics_delta_upwards(metrics_delta, it.index);
    }
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::notify_update_state(u32_t modification_stamp, iterator it, state_diff_t & diff) {
    J_ASSERT(is_leaf() && it.node == this && it.index < size(), "Iterator mismatch");
    while (diff.should_propagate() && it) {
      it->ref.set_modification_stamp(modification_stamp);
      it->update_state(diff);
      it.node->notify_metrics_delta(it, diff);
      if (!diff.should_propagate()) {
        break;
      }
      ++it;
    }
  }

  template<RopeTreeDescription Desc>
  typename rope_tree_node<Desc>::iterator rope_tree_node<Desc>::erase_middle(
    u32_t modification_stamp,
    iterator it,
    key_t start,
    key_t len
  ) {
    J_ASSERT(it.node == this && is_leaf(), "Node mismatch");
    const auto idx = it.index;
    state_diff_t chg(previous_diff(idx));
    pair<value_t, metrics_t> res = value_at(idx).erase_middle(chg, start, len);
    res.first.ref.set_modification_stamp(modification_stamp);
    J_ASSERT(chg.metrics_delta().key() == -(key_diff_t)len, "Invalid erased value");
    apply_metrics_delta_upwards(chg.metrics_delta(), idx);
    const auto sz = size();
    if (sz != max_size_v) {
      emplace_value_at_no_metrics(idx + 1U, static_cast<value_t &&>(res.first));
      m_metrics[idx] -= res.second;
      ++it.index;
      return it;
    }

    for (u8_t i = idx; i < sz; ++i) {
      m_metrics[i] -= res.second;
    }
    split_result split_res = split(idx + 1U, it);
    split_res.target_node->emplace_value_at_known_metrics(
      split_res.target_index,
      res.second,
      static_cast<value_t &&>(res.first));

    rope_tree_node * const p = parent();
    split_res = p ? p->handle_split_upwards(split_res, this->index())
      : handle_split_root(split_res);
    return ++split_res.tracked_iterator;
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::apply_metrics_delta_upwards(const metrics_t & delta, u8_t index) {
    if (!delta) {
      return;
    }
    const u8_t sz = size();
    J_ASSUME(index <= sz);
    for (; index != sz; ++index) {
      m_metrics[index] += delta;
    }
    if (const auto p = parent()) {
      p->apply_metrics_delta_upwards(delta, this->index());
    }
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::propagate_state_change(u32_t modification_stamp, u8_t index, state_diff_t & delta) noexcept {
    value_at(index).ref.set_modification_stamp(modification_stamp);
    auto metrics = delta.metrics_delta();
    if (metrics) {
      delta.reset_metrics_delta();
      apply_metrics_delta_upwards(metrics, index);
    }

    iterator it{this, (i8_t)index};
    if (++it) {
      it.node->notify_update_state(modification_stamp, it, delta);
    }
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::insert_node_at(u8_t index,
                                            rope_tree_node * J_NOT_NULL node,
                                            bool set_next_prev) {
    if (set_next_prev) {
      const u8_t sz = size();
      if (J_UNLIKELY(!sz)) {
        node->next = node->previous = nullptr;
      } else {
        node->next = index < sz ? m_data[index].node_ptr() : m_data[index - 1U].node_ptr()->next;
        node->previous = index ? m_data[index - 1U].node_ptr() : m_data[index].node_ptr()->previous;
        if (node->next) { node->next->previous = node; }
        if (node->previous) { node->previous->next = node; }
      }
    }
    prepare_insert_at(index, node->metrics())->set_node_ptr(node);
    node->set_parent_and_index(this, index);
  }

  template<RopeTreeDescription Desc>
  J_RETURNS_NONNULL typename rope_tree_node<Desc>::data_container *
  rope_tree_node<Desc>::prepare_insert_at(u8_t index, metrics_t inserted_metrics) {
      const u8_t sz = size();
      set_size(sz + 1U);
      if (index < sz) {
        const u8_t num_moved = sz - index;
        copy_data<true, false>(m_data + index + 1U, m_data + index, num_moved);
        ::j::memmove(m_metrics + index + 1U, m_metrics + index, num_moved * sizeof(metrics_t));
        m_metrics[index] = index ? m_metrics[index - 1U] + inserted_metrics : inserted_metrics;
        for (u8_t i = index + 1U; i < sz + 1U; ++i) {
          update_child(i);
          m_metrics[i] += inserted_metrics;
        }
      } else {
        J_ASSUME(index == sz);
        m_metrics[index] = m_metrics[index - 1U] + inserted_metrics;
      }

      return m_data + index;
    }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::compact_at(u8_t index, metrics_t metrics_delta,
                                        bool set_next_prev, bool destroy) {
    u8_t sz = size();
    J_ASSUME(index < sz);
    set_size(--sz);
    if (set_next_prev && is_inner()) {
      auto n = m_data[index].node_ptr();
      if (n->previous) {
        n->previous->next = n->next;
      }
      if (n->next) {
        n->next->previous = n->previous;
      }
    }
    if (destroy) {
      if (is_inner()) {
        ::delete m_data[index].node_ptr();
      } else {
        m_data[index].destroy_value();
      }
    }
    copy_data<true, false>(m_data + index, m_data + index + 1U, sz - index);
    for (; index < sz; ++index) {
      m_metrics[index] = m_metrics[index + 1U] + metrics_delta;
      update_child(index);
    }
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::merge_with(rope_tree_node * J_NOT_NULL J_RESTRICT next) {
    const u8_t sz = size();
    const u8_t next_sz = next->size();
    J_ASSUME(sz <= max_size_v / 2);
    J_ASSUME(next_sz <= max_size_v / 2);
    J_ASSUME(sz >= 1);
    J_ASSUME(next_sz >= 1);
    const metrics_t & metrics_delta = metrics();
    set_size(sz + next_sz);
    copy_data<true, true>(m_data + sz, next->m_data, next_sz);
    for (u8_t i = 0U; i < next_sz; ++i) {
      m_metrics[sz + i] = next->m_metrics[i] + metrics_delta;
      update_child(sz + i);
    }
    this->next = next->next;
    if (this->next) {
      this->next->previous = this;
    }
    ::delete next;
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::balance(u8_t to_index, u8_t from_index) {
    J_ASSUME(to_index != from_index);
    auto & from = node_at(from_index);
    auto & to = node_at(to_index);
    const u8_t from_sz = from.size();
    J_ASSUME(from_sz > max_size_v / 2U);
    J_ASSERT(is_inner() && to.size() == max_size_v / 2U - 1U,
             "Invalid arguments");
    const u8_t to_subindex = to_index < from_index ? max_size_v / 2U - 1U : 0U;
    const u8_t from_subindex = to_index < from_index ? 0U : from_sz - 1U;
    const metrics_t metrics_delta = from.metrics_delta(from_subindex);
    auto v = to.prepare_insert_at(to_subindex, metrics_delta);
    if (to.is_inner()) {
      v->set_node_ptr(from.m_data[from_subindex].node_ptr());
      v->node_ptr()->set_parent_and_index(&to, to_subindex);
    } else {
      auto val = v->construct_value(static_cast<value_t &&>(*from.m_data[from_subindex].value_ptr()));
      val->ref.set(&to, to_subindex);
    }
    from.compact_at(from_subindex, -metrics_delta, false);
    if (to_index < from_index) {
      m_metrics[to_index] += metrics_delta;
    } else {
      m_metrics[from_index] -= metrics_delta;
    }
  }

  template<RopeTreeDescription Desc>
  u32_t rope_tree_node<Desc>::depth() const noexcept {
    if (is_leaf()) {
      return 1U;
    }
    J_ASSERT_NOT_NULL(size(), m_data[0].node_ptr());
    return m_data[0].node_ptr()->depth() + 1U;
  }

  template<RopeTreeDescription Desc>
  typename rope_tree_node<Desc>::split_result rope_tree_node<Desc>::handle_split_upwards(
    split_result result, u8_t index)
  {
    result = handle_split(result, index);
    if (!result) {
      return result;
    }
    const auto p = parent();
    return p ? p->handle_split_upwards(result, this->index())
              : handle_split_root(result);
  }

  template<RopeTreeDescription Desc>
  [[nodiscard]] typename rope_tree_node<Desc>::split_result rope_tree_node<Desc>::split(
    u8_t index, iterator tracked_iterator) {
    J_ASSERT(size() == max_size_v, "Split when not full");

    constexpr u8_t half_sz = max_size_v >> 1;
    set_size(half_sz);
    rope_tree_node * const right_part = ::new rope_tree_node(m_parent_and_data);
    const metrics_t & left_metrics = m_metrics[half_sz - 1U];
    right_part->copy_data<true, true>(right_part->m_data, m_data + half_sz, half_sz);
    for (u8_t i = 0U; i < half_sz; ++i) {
      right_part->m_metrics[i] = m_metrics[i + half_sz] - left_metrics;
      right_part->update_child(i);
    }

    const bool insert_left = index <= half_sz;
    rope_tree_node * target_node = (insert_left ? this : right_part);
    u8_t target_index = insert_left ? index : index - half_sz;
    if (tracked_iterator.node == this && tracked_iterator.index >= size()) {
      tracked_iterator = {right_part, (i8_t)(tracked_iterator.index - size())};
    }
    return split_result{right_part, tracked_iterator, target_node, target_index};
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::update_metrics(u8_t index) {
    const u8_t sz = size();
    J_ASSUME(index < sz);
    auto metrics_delta = node_at(index).metrics() - m_metrics[index];
    if (index) {
      metrics_delta += m_metrics[index - 1U];
    }
    for (; index < sz; ++index) {
      m_metrics[index] += metrics_delta;
    }
  }

  template<RopeTreeDescription Desc>
  typename rope_tree_node<Desc>::split_result rope_tree_node<Desc>::handle_split(split_result result, u8_t index) {
    J_ASSERT(is_inner(), "Handle split must be called for inner nodes only");
    if (!result) {
      return result;
    }
    update_metrics(index);
    if (size() == max_size_v) {
      split_result split_res = split(index + 1U, result.tracked_iterator);
      split_res.target_node->insert_node_at(split_res.target_index, result.split_node);
      return split_res;
    }
    insert_node_at(index + 1U, result.split_node);
    if (result.tracked_iterator.node == this && result.tracked_iterator.index > index + 1) {
      ++result.tracked_iterator.index;
    }
    result.split_node = nullptr;
    return result;
  }

  template<RopeTreeDescription Desc>
  typename rope_tree_node<Desc>::split_result rope_tree_node<Desc>::handle_split_root(split_result result) {
    J_ASSERT(!parent(), "handle_split_root called for non-root.");
    if (result) {
      rope_tree_node * const split_left = ::new rope_tree_node(static_cast<rope_tree_node &&>(*this));
      *this = rope_tree_node(split_left, result.split_node);
      if (result.tracked_iterator.node == this) {
        result.tracked_iterator.node = split_left;
      }
      result.split_node = nullptr;
    }
    return result;
  }

  template<RopeTreeDescription Desc>
  void rope_tree_node<Desc>::copy_from(rope_tree_node & J_RESTRICT rhs) noexcept {
    ::j::memcpy(this, &rhs, sizeof(rope_tree_node));
    const u8_t sz = ::j::max(size(), 1);
    copy_data<true, true>(m_data, rhs.m_data, sz);
    for (u8_t i = 0U; i < sz; ++i) {
      update_child(i);
    }
    if (next) {
      next->previous = this;
    }
    if (previous) {
      previous->next = this;
    }
  }

  template<RopeTreeDescription DescriptionT>
  typename rope_tree_node<DescriptionT>::iterator rope_tree_node<DescriptionT>::erase(state_diff_t & diff, const_iterator it_) {
    const u8_t sz = size();
    J_ASSUME(sz > 0);
    J_ASSERT(it_.node == this && is_leaf(), "Iterator mismatch");
    const u8_t idx = (u8_t)it_.index;
    J_ASSUME(idx < sz);
    iterator it = {this, (i8_t)idx};
    it->erase(diff);
    auto metrics_delta = diff.metrics_delta();
    diff.reset_metrics_delta();
    if (sz == 1) {
      set_size(0);
      return it;
    }
    compact_at(idx, metrics_delta, true, true);
    if (it.index == sz - 1 && next) {
      it = {next, 0};
    }
    if (const auto p = parent()) {
      return p->handle_erase_upwards(
        {sz == max_size_v / 2U, metrics_delta, it},
        this->index()).tracked_iterator;
    }
    return it;
  }

  template<RopeTreeDescription DescriptionT>
  typename rope_tree_node<DescriptionT>::erase_result
  rope_tree_node<DescriptionT>::handle_erase(erase_result result, u8_t index) {
    const u8_t sz = size();
    J_ASSUME(sz > 0);
    J_ASSUME(index < sz);
    for (u8_t i = index; i < sz; ++i) {
      m_metrics[i] += result.metrics_delta;
    }
    if (!result.is_below_minimum) {
      return result;
    }
    rope_tree_node * const current = &node_at(index);
    rope_tree_node * const previous = index ? &node_at(index - 1U) : nullptr;
    rope_tree_node * const next = index != sz - 1U ? &node_at(index + 1U) : nullptr;
    if (!previous && !next) {
      return result;
    }
    if (previous && previous->size() > max_size_v / 2U) {
      balance(index, index - 1U);
      result.is_below_minimum = false;
      if (result.tracked_iterator.node == current) {
        ++result.tracked_iterator.index;
      } else if (result.tracked_iterator.node == previous
                  && result.tracked_iterator.index == previous->size() - 1) {
        result.tracked_iterator = {current, 0};
      }
    } else if (next && next->size() > max_size_v / 2U) {
      balance(index, index + 1U);
      result.is_below_minimum = false;
      if (result.tracked_iterator.node == next) {
        if (result.tracked_iterator.index == 0) {
          result.tracked_iterator = {current, (i8_t)(current->size() - 1)};
        } else {
          --result.tracked_iterator.index;
        }
      }
    } else {
      if (next) {
        if (result.tracked_iterator.node == next) {
          result.tracked_iterator.node = current;
          result.tracked_iterator.index += current->size();
        }
        m_metrics[index] = m_metrics[index + 1U];
        compact_at(index + 1U, {});
        current->merge_with(next);
      } else {
        if (result.tracked_iterator.node == current) {
          result.tracked_iterator.node = previous;
          result.tracked_iterator.index += previous->size();
        }
        m_metrics[index - 1U] = m_metrics[index];
        compact_at(index, {});
        previous->merge_with(current);
      }
      result.is_below_minimum = sz == max_size_v / 2U;
    }
    return result;
  }

  template<RopeTreeDescription DescriptionT>
  typename rope_tree_node<DescriptionT>::erase_result
  rope_tree_node<DescriptionT>::handle_erase_root(erase_result result) {
    J_ASSERT(!parent(), "Not root.");
    const u8_t sz = size();
    if (is_inner() && sz == 1U) {
      auto n = &node_at(0);
      if (result.tracked_iterator.node == n) {
        result.tracked_iterator.node = this;
      }
      m_data[0].set_node_ptr(nullptr);
      *this = static_cast<rope_tree_node &&>(*n);
      ::delete n;
      set_parent_and_index(nullptr, 0U);
    }
    return result;
  }

  template<RopeTreeDescription DescriptionT>
  typename rope_tree_node<DescriptionT>::erase_result
  rope_tree_node<DescriptionT>::handle_erase_upwards(erase_result result, u8_t index) {
    result = handle_erase(result, index);
    const auto p = parent();
    return p
      ? p->handle_erase_upwards(result, this->index())
      : handle_erase_root(result);
  }
}
