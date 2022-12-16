#include "properties/access/set_access.hpp"

namespace j::properties::access {
  set_iterator set_access::begin() {
    return {&def(), def().m_begin(as_void_star())};
  }

  set_iterator set_access::end() {
    return {&def(), def().m_end(as_void_star())};
  }

  set_iterator set_access::find(const typed_access & key) {
    return {&def(), def().m_find(as_void_star(), key)};
  }

  pair<set_iterator, bool> set_access::insert(const typed_access & item) {
    auto result = def().m_insert(as_void_star(), item);
    return {
      {&def(), static_cast<iterator_data &&>(result.first)}, result.second};
  }

  set_iterator set_access::erase(const set_iterator & it) {
    return {&def(), def().m_erase_by_iterator(as_void_star(), it.data())};
  }
}
