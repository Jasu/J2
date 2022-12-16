#pragma once

#include "properties/access/map_access_definition.hpp"
#include "properties/access/map_access_iterator.hpp"
#include "properties/access/typed_access.hpp"

namespace j::inline containers {
  template<typename, typename> struct pair;
}

namespace j::properties::access {
  class map_access final : public typed_access {
    const map_access_definition & def() const noexcept {
      return static_cast<const map_access_definition &>(typed_access::value_definition());
    }
  public:
    using typed_access::typed_access;

    const wrappers::wrapper_definition & key_definition() const {
      return def().value_definition();
    }

    const wrappers::wrapper_definition & value_definition() const {
      return def().value_definition();
    }

    bool empty() const noexcept {
      return def().m_empty(as_void_star());
    }

    map_iterator begin();

    map_iterator end();

    map_iterator find(const typed_access & key);

    sz_t size() const noexcept {
      return def().m_get_size(as_void_star());
    }

    wrappers::wrapper at(const typed_access & key);

    wrappers::wrapper operator[](const typed_access & key);

    pair<map_iterator, bool> insert(const typed_access & key, const typed_access & value);

    map_iterator replace(const typed_access & key, const typed_access & value);

    map_iterator erase(const map_iterator & it);

    sz_t erase(const typed_access & key) {
      return def().m_erase_by_key(as_void_star(), key);
    }

    void clear() {
      return def().m_clear(as_void_star());
    }
  };
}
