#pragma once

#include "properties/access/list_access_definition.hpp"
#include "properties/access/typed_access.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/access/lists/list_iterator.hpp"

namespace j::properties::access {
  class list_access final : public typed_access {
    const list_access_definition & def() const noexcept {
      return static_cast<const list_access_definition &>(value_definition());
    }
  public:
    using typed_access::typed_access;

    const wrappers::wrapper_definition & item_definition() const {
      return *def().item_definition();
    }

    bool empty() const noexcept {
      return def().m_empty(as_void_star());
    }

    sz_t size() const noexcept {
      return def().m_get_size(as_void_star());
    }

    wrappers::wrapper at(sz_t index) {
      return def().m_at(as_void_star(), index);
    }

    wrappers::wrapper operator[](sz_t index) {
      return def().m_at(as_void_star(), index);
    }

    void erase(sz_t i) {
      return def().m_erase_by_index(as_void_star(), i);
    }

    list_iterator erase(list_iterator it) {
      return def().m_erase_by_iterator(as_void_star(), it);
    }

    void clear() {
      return def().m_clear(as_void_star());
    }

    list_iterator begin() {
      return def().m_begin(as_void_star());
    }

    list_iterator end() {
      return def().m_end(as_void_star());
    }

    void push_back(const typed_access & value) {
      def().m_push_back(as_void_star(), value);
    }

    list_iterator insert(list_iterator it, const typed_access & value) {
      return def().m_insert(as_void_star(), it, value);
    }
  };
}
