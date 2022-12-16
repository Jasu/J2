#pragma once

#include "containers/trees/rope_tree_concepts.hpp"
#include "containers/trees/rope_tree_iterator.hpp"
#include "containers/pair.hpp"
#include "containers/trees/rope_tree_ref.hpp"

namespace j::inline containers::trees {
  template<RopeTreeDescription Description>
  class rope_tree_node final {
    static constexpr inline uptr_t size_mask_v     = 0x00FFULL;
    static constexpr inline uptr_t index_mask_v    = 0x7F00ULL;
    static constexpr inline uptr_t is_inner_mask_v = 0x8000ULL;

    class data_container;

  public:
    static constexpr inline u8_t max_size_v = Description::max_size_v;

    using key_t = typename Description::key_t;
    using key_diff_t = typename Description::key_diff_t;
    using metrics_t = typename Description::metrics_t;
    using value_t = typename Description::value_t;
    using controller_t = typename Description::controller_t;
    using iterator = rope_tree_iterator<rope_tree_node, false>;
    using state_diff_t =  typename Description::state_diff_t;
    using const_iterator = rope_tree_iterator<rope_tree_node, true>;

    static const inline key_t null_key_v{};
    static const inline metrics_t null_metrics_v{};

    [[no_unique_address]] controller_t m_controller;

    rope_tree_node * next = nullptr;
    rope_tree_node * previous = nullptr;

    struct split_result final {
      rope_tree_node * split_node = nullptr;
      iterator tracked_iterator;
      rope_tree_node * target_node = nullptr;
      u8_t target_index = 0xFF;
      explicit operator bool() const noexcept {
        return split_node;
      }
    };

    J_ALWAYS_INLINE rope_tree_node() noexcept {
      m_data[0].construct_value()->ref.set(this, 0);
    }

    void destroy() noexcept {
      u8_t sz = size();
      if (is_inner()) {
        for (u8_t i = 0U; i < sz; ++i) {
          ::delete m_data[i].node_ptr();
        }
      } else {
        sz = sz ? sz : 1U;
        for (u8_t i = 0U; i < sz; ++i) {
          m_data[i].destroy_value();
        }
      }
    }

    ~rope_tree_node() {
      destroy();
    }

    rope_tree_node(rope_tree_node && rhs) noexcept;

    rope_tree_node & operator=(rope_tree_node && rhs) noexcept;

    rope_tree_node(rope_tree_node * J_NOT_NULL J_RESTRICT left_child,
                   rope_tree_node * J_NOT_NULL J_RESTRICT right_child) noexcept;

    /// Get the number of children in the node.
    J_A(AI,NODISC,NE) u8_t size() const noexcept { return m_parent_and_data; }

    J_A(AI,NODISC,NE) bool empty() const noexcept
    { return !(u8_t)m_parent_and_data; }

    /// Get the summed key of the node.
    [[nodiscard]] const metrics_t & metrics() const noexcept {
      const u8_t sz = size();
      return J_UNLIKELY(!sz) ? null_metrics_v : metrics_at(sz - 1U);
    }

    J_INLINE_GETTER rope_tree_node * parent() const noexcept
    { return reinterpret_cast<rope_tree_node *>(m_parent_and_data >> 16); }

    /// Get the index of the node in its parent.
    J_INLINE_GETTER u8_t index() const noexcept { return (m_parent_and_data & index_mask_v) >> 8; }

    J_INLINE_GETTER bool is_inner() const noexcept { return m_parent_and_data & is_inner_mask_v; }
    J_INLINE_GETTER bool is_leaf() const noexcept { return !(m_parent_and_data & is_inner_mask_v); }

    [[nodiscard]] value_t & value_at(u8_t i) noexcept {
      J_ASSERT(i < max_size_v);
      return *(m_data[i].value_ptr());
    }

    J_INLINE_GETTER const value_t & value_at(u8_t i) const noexcept
    { return const_cast<rope_tree_node *>(this)->value_at(i); }

    [[nodiscard]] rope_tree_node & node_at(u8_t i) noexcept {
      J_ASSERT(i < max_size_v);
      return *m_data[i].node_ptr();
    }

    J_INLINE_GETTER const rope_tree_node & node_at(u8_t i) const noexcept
    { return const_cast<rope_tree_node *>(this)->node_at(i); }

    J_A(AI,NODISC,NE) const metrics_t & metrics_at(u8_t i) const noexcept {
      return m_metrics[i];
    }

    [[nodiscard]] const metrics_t & metrics_before(u8_t i) const noexcept {
      J_ASSERT(i < max_size_v);
      return i ? m_metrics[i - 1U] : null_metrics_v;
    }

    [[nodiscard]] metrics_t cumulative_metrics(u8_t i) const noexcept {
      J_ASSERT(i < max_size_v);
      const auto k = metrics_before(i);
      const auto p = parent();
      return p ? p->cumulative_metrics(index()) + k : k;
    }

    [[nodiscard]] metrics_t metrics_delta(u8_t i) const noexcept
    { return i ? metrics_at(i) - m_metrics[i - 1U] : metrics_at(i); }

    [[nodiscard]] key_t position(u8_t i) const noexcept {
      const auto p = parent();
      const key_t key = metrics_at(i).key();
      return p ? p->position(index()) + key : key;
    }

    [[nodiscard]] key_diff_t key_delta(u8_t i) const noexcept
    { return metrics_at(i).key() - (i ? m_metrics[i - 1U].key() : 0); }

    /// Get the depth of the node (leaf nodes have depth=1).
    u32_t depth() const noexcept;

    [[nodiscard]] iterator begin() noexcept
    { return is_leaf() ? iterator{this, 0} : node_at(0).begin(); }

    [[nodiscard]] iterator end() noexcept {
      const u8_t sz = size();
      return is_leaf() ? iterator{this, (i8_t)sz} : node_at(sz - 1U).end();
    }

    [[nodiscard]] iterator before_end() noexcept {
      const i8_t last = ::j::max(0, size() - 1U);
      return is_leaf() ? iterator{this, last} : node_at(last).before_end();
    }

    /// Find the first node that compares greater than or equal to key.
    pair<iterator, key_t> find(const key_t key);

    template<typename... Args>
    iterator split_emplace_value_upwards(u32_t modification_stamp, u8_t index, Args && ... args) {
      auto res = split(index, {this, (i8_t)index});
      res.tracked_iterator.node->emplace_value_at(
        modification_stamp, res.tracked_iterator.index, static_cast<Args &&>(args)...);
      if (rope_tree_node * const p = parent()) {
        return p->handle_split_upwards(res, this->index()).tracked_iterator;
      } else {
        return handle_split_root(res).tracked_iterator;
      }
    }

    iterator split_at(u8_t index, key_t offset);

    template<typename... Args>
    iterator emplace_before(u32_t modification_stamp, iterator it, Args && ... args) {
      const u32_t sz = size();
      J_ASSUME((u8_t)it.index <= sz);
      J_ASSERT(it.node == this && is_leaf(), "Invalid emplace_before arguments");
      if (sz != max_size_v) {
        emplace_value_at(modification_stamp, it.index, static_cast<Args &&>(args)...);
        return it;
      }
      return split_emplace_value_upwards(modification_stamp, it.index, static_cast<Args &&>(args)...);
    }

    template<typename... Args>
    iterator emplace_at(u32_t modification_state, iterator it, key_t offset, Args && ... args) {
      J_ASSERT(it.node == this && is_leaf(), "Iterator mismatch");
      if (offset) {
        const i8_t sz = size() - 1;
        J_ASSUME(sz >= 0);
        if (it.index == sz && offset == (key_t)key_delta(sz)) {
          ++it.index;
        } else {
          it = split_at(it.index, offset);
          ++it;
        }
      }
      return it.node->emplace_before(modification_state, it, static_cast<Args &&>(args)...);
    }

    iterator erase(state_diff_t & diff, const_iterator it);

    void notify_metrics_delta(iterator it, state_diff_t & diff);

    void notify_update_state(u32_t modification_stamp, iterator it, state_diff_t & diff);

    iterator erase_middle(u32_t modification_stamp, iterator it, key_t start, key_t len);

    void apply_metrics_delta_upwards(const metrics_t & delta, u8_t index);

    /// Notify that the metrics of the node have changed by delta.
    void propagate_state_change(u32_t modification_stamp, u8_t index, state_diff_t & delta) noexcept;

    template<typename OtherKeyT>
    pair<iterator, key_t> find_by_secondary(OtherKeyT key) {
      u8_t i = 0U;
      const u8_t sz = size();

      for (; i < sz && m_controller.secondary_key_less_than(m_metrics[i], key); ++i) { }
      J_REQUIRE(i < sz, "Out of bounds");
      if (i) {
        key = m_controller.secondary_key_diff(key, m_metrics[i - 1U]);
      }
      if (is_inner()) {
        return node_at(i).template find_by_secondary<OtherKeyT>(key);
      } else {
        return {
          iterator{ this, (i8_t)i },
          m_controller.secondary_key_to_index(key, value_at(i))
        };
      }
    }


  private:
    struct erase_result final {
      bool is_below_minimum;
      metrics_t metrics_delta;
      iterator tracked_iterator;
    };

    erase_result handle_erase(erase_result result, u8_t index);
    erase_result handle_erase_root(erase_result result);
    erase_result handle_erase_upwards(erase_result result, u8_t index);

    template<typename... Args>
    J_RETURNS_NONNULL value_t * emplace_value_at_no_metrics(u8_t index, Args && ... args) {
      auto v = prepare_insert_at(index, {})->construct_value(static_cast<Args &&>(args)...);
      v->ref.set(this, index);
      return v;
    }

    template<typename... Args>
    J_RETURNS_NONNULL value_t * emplace_value_at_known_metrics(u8_t index, const metrics_t & metrics, Args && ... args) {
      value_t * v = emplace_value_at_no_metrics(index, static_cast<Args &&>(args)...);
      for (const u8_t sz = size(); index < sz; ++index) {
        m_metrics[index] += metrics;
      }
      return v;
    }

    state_diff_t previous_diff(u8_t index) const noexcept {
      J_ASSUME(index < max_size_v);
      return state_diff_t(
        index ? &value_at(index - 1U) :
        (previous
         ? &previous->value_at(previous->size() - 1U)
         : nullptr));
    }

    template<typename... Args>
    void emplace_value_at(u32_t modification_stamp, u8_t index, Args && ... args) {
      if (index == 0 && size() == 0) {
        set_size(1);
        auto v = m_data[0].value_ptr();
        v->initialize(static_cast<Args &&>(args)...);
        state_diff_t diff;
        v->initialize_state(diff);
        m_metrics[0] = diff.metrics_delta();
        return;
      }
      value_t * v = emplace_value_at_no_metrics(index, static_cast<Args &&>(args)...);
      state_diff_t diff(previous_diff(index));
      v->initialize_state(diff);
      propagate_state_change(modification_stamp, index, diff);
    }

    void insert_node_at(u8_t index, rope_tree_node * J_NOT_NULL node, bool set_next_prev = true);

    J_RETURNS_NONNULL data_container * prepare_insert_at(u8_t index, metrics_t inserted_metrics);

    void compact_at(u8_t index, metrics_t metrics_delta, bool set_next_prev = true, bool destroy = false);

    void merge_with(rope_tree_node * J_NOT_NULL J_RESTRICT next);

    [[nodiscard]] split_result split(u8_t index, iterator tracked_iterator);

    void update_metrics(u8_t index);

    split_result handle_split(split_result result, u8_t index);

    split_result handle_split_upwards(split_result result, u8_t index);

    split_result handle_split_root(split_result result);

    /// Balance the tree after a deletion by moving a single node from from_index to to_index.
    void balance(u8_t to_index, u8_t from_index);

    void copy_from(rope_tree_node & J_RESTRICT rhs) noexcept;

    J_ALWAYS_INLINE explicit rope_tree_node(uptr_t parent_and_data) noexcept
      : m_parent_and_data(parent_and_data)
    { }

    void set_size(u8_t sz) noexcept {
      J_ASSERT(sz <= max_size_v);
      m_parent_and_data = (m_parent_and_data & ~size_mask_v) | (uptr_t)sz;
    }

    void update_child(u8_t index) noexcept {
      J_ASSERT(index < max_size_v);
      is_inner()
        ? node_at(index).set_parent_and_index(this, index)
        : value_at(index).ref.set(this, index);
    }

    void set_parent_and_index(rope_tree_node * p, u8_t index) noexcept {
      J_ASSERT(index < max_size_v);
      m_parent_and_data = (m_parent_and_data & 0x80FFULL) | ((uptr_t)p << 16) | ((uptr_t)index << 8);
    }

    uptr_t m_parent_and_data = 0ULL;
    metrics_t m_metrics[max_size_v];

    class data_container final {
      alignas(value_t) u8_t m_buffer[::j::max(sizeof(value_t), sizeof(void*))] = {0U};
    public:
      J_A(AI,NODISC,NE) value_t * value_ptr() noexcept
      { return reinterpret_cast<value_t*>(m_buffer); }

      J_A(AI,NODISC,NE) const value_t * value_ptr() const noexcept
      { return reinterpret_cast<const value_t*>(m_buffer); }

      J_A(AI,NODISC,NE) rope_tree_node * node_ptr() noexcept
      { return *reinterpret_cast<rope_tree_node**>(&m_buffer); }

      J_A(AI,NODISC,NE) const rope_tree_node * node_ptr() const noexcept
      { return *reinterpret_cast<const rope_tree_node* const *>(&m_buffer); }

      J_A(AI,NE) void set_node_ptr(rope_tree_node * ptr) noexcept
      { *reinterpret_cast<rope_tree_node**>(&m_buffer) = ptr; }

      template<typename... Args>
      J_A(AI,NE,ND) value_t * construct_value(Args && ... args)
      { return ::new (value_ptr()) value_t(static_cast<Args &&>(args)...); }

      void destroy_value() noexcept
      { value_ptr()->~value_t(); }
    };

    data_container m_data[max_size_v];

    template<bool Move, bool Initialize>
    void copy_data(data_container * J_NOT_NULL target, data_container * J_NOT_NULL source, u8_t count) {
      J_ASSUME(target != source);
      const i32_t dir = target < source ? 1 : -1;
      if (target > source) {
        target += count - 1U;
        source += count - 1U;
      }

      const bool inner = is_inner();
      for (u8_t i = 0; i < count; ++i, target += dir, source += dir) {
        J_ASSERT(target >= m_data + 0 && target < m_data + max_size_v, "Out of bounds.");
        if (inner) {
          target->set_node_ptr(source->node_ptr());
          if constexpr (Move) {
            source->set_node_ptr(nullptr);
          }
        } else {
          if constexpr (Move) {
            if constexpr (Initialize) {
              ::new (target->value_ptr()) value_t(static_cast<value_t &&>(*(source->value_ptr())));
            } else {
              *(target->value_ptr()) = static_cast<value_t &&>(*(source->value_ptr()));
            }
          } else {
            if constexpr (Initialize) {
              ::new (target->value_ptr()) value_t(*(source->value_ptr()));
            } else {
              *(target->value_ptr()) = *(source->value_ptr());
            }
          }
        }
      }
    }
  };
}
