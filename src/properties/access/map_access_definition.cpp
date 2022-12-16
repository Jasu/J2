#include "properties/access/map_access_definition.hpp"
#include "properties/access/map_access.hpp"
#include "containers/pair.hpp"

namespace j::properties::access {
  map_access_definition::map_access_definition() noexcept {
  }

  map_access_definition::~map_access_definition() {
  }

  void map_access_definition::initialize_map_access(
    get_sz_t get_size,
    empty_t empty,
    at_t at,
    begin_t begin,
    end_t end,
    find_t find,

    get_iterator_key_t get_iterator_key,
    get_iterator_value_t get_iterator_value,
    release_iterator_t release_iterator,
    advance_iterator_t advance_iterator,
    copy_iterator_t copy_iterator,
    iterator_equals_t iterator_equals,

    insert_t insert,
    replace_t replace,

    erase_by_key_t erase_by_key,
    erase_by_iterator_t erase_by_iterator,
    clear_t clear
  ) {
    m_get_size = get_size;
    m_empty = empty;
    m_at = at;

    m_begin = begin;
    m_end = end;
    m_find = find;

    m_get_iterator_key = get_iterator_key;
    m_get_iterator_value = get_iterator_value;
    m_release_iterator = release_iterator;
    m_advance_iterator = advance_iterator;
    m_copy_iterator = copy_iterator;
    m_iterator_equals = iterator_equals;

    m_insert = insert;
    m_replace = replace;

    m_erase_by_key = erase_by_key;
    m_erase_by_iterator = erase_by_iterator;
    m_clear = clear;

    // release iterator may be empty
    J_ASSERT_NOT_NULL(m_get_size, m_empty, m_at, m_begin, m_end);
    J_ASSERT_NOT_NULL(m_find, m_get_iterator_key, m_get_iterator_value, m_advance_iterator, m_copy_iterator);
    J_ASSERT_NOT_NULL(m_iterator_equals, m_insert, m_replace, m_clear, m_erase_by_key);
    J_ASSERT_NOT_NULL(m_erase_by_iterator);

    J_ASSERT(type == PROPERTY_TYPE::MAP,
              "Base access does not describe a map.");
  }

  map_iterator map_access::begin() {
    return {&def(), def().m_begin(as_void_star())};
  }

  map_iterator map_access::end() {
    return {&def(), def().m_end(as_void_star())};
  }

  map_iterator map_access::find(const typed_access & key) {
    return {&def(), def().m_find(as_void_star(), key)};
  }

  wrappers::wrapper map_access::at(const typed_access & key) {
    return def().m_at(as_void_star(), key);
  }

  wrappers::wrapper map_access::operator[](const typed_access & key) {
    return def().m_at(as_void_star(), key);
  }

  pair<map_iterator, bool> map_access::insert(const typed_access & key, const typed_access & value) {
    auto result = def().m_insert(as_void_star(), key, value);
    return pair<map_iterator, bool>{{&def(), static_cast<iterator_data &&>(result.first) }, result.second};
  }

  map_iterator map_access::replace(const typed_access & key, const typed_access & value) {
    return map_iterator{ &def(), def().m_replace(as_void_star(), key, value) };
  }

  map_iterator map_access::erase(const map_iterator & it) {
    return map_iterator{&def(), def().m_erase_by_iterator(as_void_star(), it.data())};
  }
}
