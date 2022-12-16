#pragma once

#include "meta/common.hpp"
#include "bits/fixed_bitset.hpp"

namespace j::meta {
  struct term;
  struct node;
  struct val_type;

  template<typename> struct node_set_iterator;
  template<typename> struct node_set_view;

  template<typename Node>
  struct node_set_iterator final {
    bits::fixed_bitset_iterator it;
    Node * const * nodes;

    J_A(AI,RNN,NODISC,ND) inline Node *operator->() const noexcept
    { return nodes[*it]; }

    J_A(AI,NODISC,ND) inline Node & operator*() const noexcept
    { return *nodes[*it]; }

    J_A(AI) inline node_set_iterator & operator++() noexcept {
      ++it;
      return *this;
    }

    inline node_set_iterator operator++(int) noexcept {
      return node_set_iterator{it++, nodes};
    }

    J_A(AI,NODISC,ND) inline bool operator==(const node_set_iterator & rhs) const noexcept {
      return it == rhs.it;
    }
  };

  template<typename Node>
  struct node_set_view final {
    const bits::bitset256 & bits;
    Node * const * nodes;

    J_A(AI,NODISC,ND) inline node_set_iterator<Node> begin() const noexcept {
      return node_set_iterator<Node>{bits.begin(), nodes};
    }
    J_A(AI,NODISC,ND) inline node_set_iterator<Node> end() const noexcept {
      return node_set_iterator<Node>{bits.end(), nullptr};
    }
  };

  struct basic_node_set {
    bits::bitset256 bitmask;

    J_A(NODISC,ND) inline u32_t size() const noexcept { return bitmask.size(); }

    J_A(NODISC,ND) inline explicit operator bool() const noexcept { return (bool)bitmask; }
    J_A(NODISC,ND) inline bool operator!() const noexcept { return !(bool)bitmask; }

    void add(const node & t) noexcept;
    void del(const node & t) noexcept;
    [[nodiscard]] bool has(const node & t) const noexcept;

    [[nodiscard]] node_set_view<const node> iterate_plain(const module * J_NOT_NULL mod, node_type t) const noexcept;

    [[nodiscard]] node_set_view<node> iterate_plain(module * J_NOT_NULL mod, node_type t) const noexcept;

    J_A(AI,NODISC,ND) inline bool operator==(const basic_node_set & rhs) const noexcept = default;

    J_A(ND) inline basic_node_set & operator-=(const basic_node_set & rhs) noexcept {
      bitmask -= rhs.bitmask;
      return *this;
    }

    J_A(NODISC,ND) inline basic_node_set operator-(const basic_node_set & rhs) const noexcept
    { return basic_node_set{ bitmask - rhs.bitmask }; }

    J_A(NODISC,ND) inline basic_node_set operator|(const basic_node_set & rhs) const noexcept
    { return basic_node_set{ bitmask | rhs.bitmask }; }

    J_A(ND) inline basic_node_set & operator|=(const basic_node_set & rhs) noexcept {
      bitmask |= rhs.bitmask;
      return *this;
    }

    J_A(NODISC,ND) inline basic_node_set operator&(const basic_node_set & rhs) const noexcept
    { return basic_node_set{ bitmask & rhs.bitmask }; }

    J_A(ND) inline basic_node_set & operator&=(const basic_node_set & rhs) noexcept {
      bitmask &= rhs.bitmask;
      return *this;
    }

    [[nodiscard]] static basic_node_set all(const module * J_NOT_NULL mod, node_type type) noexcept;
  };

  struct wrapped_node_set final {
    const basic_node_set * set = nullptr;
    node_type type = node_none;

    J_A(NODISC,ND,AI) operator const basic_node_set &() const noexcept { return *set; }

    J_A(NODISC,ND) inline u32_t size() const noexcept { return set->bitmask.size(); }

    J_A(NODISC,ND) inline explicit operator bool() const noexcept { return (bool)set->bitmask; }
    J_A(NODISC,ND) inline bool operator!() const noexcept { return !(bool)set->bitmask; }

    [[nodiscard]] bool has(const node & t) const noexcept;

    [[nodiscard]] node_set_view<const node> iterate(const module * J_NOT_NULL mod) const noexcept;
    [[nodiscard]] node_set_view<node> iterate(module * J_NOT_NULL mod) const noexcept;

    [[nodiscard]] bool operator==(const wrapped_node_set & rhs) const noexcept;
  };
}
