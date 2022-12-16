#include "basic_node_set.hpp"
#include "meta/node.hpp"
#include "meta/module.hpp"

namespace j::meta {
  void basic_node_set::add(const node & t) noexcept {
    bitmask.add(t.index);
  }
  void basic_node_set::del(const node & t) noexcept {
    bitmask.del(t.index);
  }
  [[nodiscard]] bool basic_node_set::has(const node & t) const noexcept {
    return bitmask.has(t.index);
  }
  [[nodiscard]] node_set_view<const node> basic_node_set::iterate_plain(const module * J_NOT_NULL mod, node_type t) const noexcept {
    return node_set_view<const node>{bitmask, reinterpret_cast<const node* const *>(mod->nodes_by_type(t).begin())};
  }

  [[nodiscard]] node_set_view<node> basic_node_set::iterate_plain(module * J_NOT_NULL mod, node_type t) const noexcept {
    return node_set_view<node>{bitmask, reinterpret_cast<node* const *>(mod->nodes_by_type(t).begin())};
  }

  [[nodiscard]] node_set_view<const node> wrapped_node_set::iterate(const module * J_NOT_NULL mod) const noexcept {
    return set->iterate_plain(mod, type);
  }

  [[nodiscard]] node_set_view<node> wrapped_node_set::iterate(module * J_NOT_NULL mod) const noexcept {
    return set->iterate_plain(mod, type);
  }

  [[nodiscard]] bool wrapped_node_set::has(const node & t) const noexcept {
    return t.type == type && set->bitmask.has(t.index);
  }

  [[nodiscard]] basic_node_set basic_node_set::all(const module * J_NOT_NULL mod, node_type type) noexcept {
    basic_node_set result;
    for (auto & n : mod->nodes_by_type(type)) {
      result.bitmask.add(n->index);
    }
    return result;
  }

  [[nodiscard]] bool wrapped_node_set::operator==(const wrapped_node_set & rhs) const noexcept {
    return *set == *rhs.set && type == rhs.type;
  }
}
