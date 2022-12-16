#pragma once

#include "properties/access/set_access_definition.hpp"
#include "properties/access/set_access_iterator.hpp"
#include "properties/access/typed_access.hpp"
#include "containers/pair.hpp"

namespace j::properties::access {
  class set_access final : public typed_access {
    const set_access_definition & def() const noexcept {
      return static_cast<const set_access_definition &>(value_definition());
    }
  public:
    using typed_access::typed_access;

    const wrappers::wrapper_definition & item_definition() const {
      return def().item_definition();
    }

    bool empty() const noexcept {
      return def().m_empty(as_void_star());
    }

    set_iterator begin();

    set_iterator end();

    set_iterator find(const typed_access & key);

    sz_t size() const noexcept {
      return def().m_get_size(as_void_star());
    }

    bool contains(const typed_access & key) {
      return def().m_contains(as_void_star(), key);
    }

    pair<set_iterator, bool> insert(const typed_access & item);

    set_iterator erase(const set_iterator & it);

    sz_t erase(const typed_access & item) {
      return def().m_erase_by_item(as_void_star(), item);
    }

    void clear() {
      return def().m_clear(as_void_star());
    }
  };
}
