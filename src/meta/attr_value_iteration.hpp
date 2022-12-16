#pragma once

#include "meta/module.hpp"
#include "meta/attr_value.hpp"
#include "meta/basic_node_set.hpp"

namespace j::meta {
  struct attr_value_iterator final {
    const attr_value * tuple_it = nullptr;
    node_set_iterator<node> set_it{};

    [[nodiscard]] attr_value operator*() const noexcept {
      return tuple_it ? *tuple_it : attr_value(&*set_it);
    }

    attr_value_iterator & operator++() noexcept {
      if (tuple_it) {
        ++tuple_it;
      } else {
        ++set_it;
      }
      return *this;
    }

    attr_value_iterator operator++(int) noexcept {
      return tuple_it
        ? attr_value_iterator{tuple_it++}
        : attr_value_iterator{nullptr, set_it++};
    }

    [[nodiscard]] bool operator==(const attr_value_iterator & rhs) const noexcept {
      return tuple_it == rhs.tuple_it && (tuple_it || set_it == rhs.set_it);
    }
  };

  struct attr_value_view final {
    attr_value * val;
    module * mod;

    [[nodiscard]] attr_value_iterator begin() const noexcept {
      return val->is_node_set()
        ? attr_value_iterator{nullptr, {val->node_set.bitmask.begin(), mod->nodes_by_type(node_set_type(val->type)).begin()}}
        : attr_value_iterator{val->type == attr_tuple ? val->tuple->begin() : val};
    }

    [[nodiscard]] attr_value_iterator end() const noexcept {
      return val->is_node_set()
        ? attr_value_iterator{nullptr, {val->node_set.bitmask.end(), nullptr}}
        : attr_value_iterator{val->type == attr_tuple ? val->tuple->end() : val + 1U};
    }
  };

  [[nodiscard]] inline attr_value_view iterate(attr_value * J_NOT_NULL val, module * J_NOT_NULL mod) {
    J_REQUIRE(to_attr_mask(val->type) & attr_mask_collection, "Value is not iterable.", val->type);
    return attr_value_view{val, mod};
  }

  [[nodiscard]] inline attr_value_view iterate_maybe_flatten(attr_value * J_NOT_NULL val, module * J_NOT_NULL mod) noexcept {
    return attr_value_view{val, mod};
  }
}
