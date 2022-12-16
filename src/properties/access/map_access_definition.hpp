#pragma once

#include "properties/access/typed_access.hpp"
#include "exceptions/assert_lite.hpp"
#include "attributes/attribute_map.hpp"
#include "containers/pair.hpp"

namespace j::properties::wrappers {
  class wrapper_definition;
  class wrapper;
}

namespace j::properties::access {
  class typed_access_definition;
  class iterator_data;

  class map_iterator;

  class map_access_definition : public typed_access_definition {
    friend class map_access;
    friend class map_iterator;
    friend class map_iterator_pair;
  public:
    using get_sz_t = sz_t (*)(const void * map) noexcept;
    using empty_t = bool (*)(const void * map) noexcept;
    using at_t = wrappers::wrapper (*)(void * map, const typed_access & key);
    using clear_t = void (*)(void * map);

    using begin_t = iterator_data (*) (void * map);
    using end_t = iterator_data (*) (void * map);
    using find_t = iterator_data (*) (void * map, const typed_access & key);

    using erase_by_key_t = sz_t (*) (void * map, const typed_access & key);
    using erase_by_iterator_t = iterator_data (*) (void * map, const iterator_data & it);

    using get_iterator_key_t = typed_access (*) (iterator_data & it);
    using get_iterator_value_t = wrappers::wrapper (*)(iterator_data & it);
    using release_iterator_t = void (*)(iterator_data & it);
    using advance_iterator_t = void (*)(iterator_data & it);
    using copy_iterator_t = iterator_data (*)(const iterator_data & it);
    using iterator_equals_t = bool (*)(const iterator_data & lhs, const iterator_data & rhs);

    using insert_t = pair<iterator_data, bool> (*)(void * map, const typed_access & key, const typed_access & value);
    using replace_t = iterator_data (*)(void * map, const typed_access & key, const typed_access & value);
  private:
    get_sz_t m_get_size = nullptr;
    empty_t m_empty = nullptr;
    at_t m_at = nullptr;

    begin_t m_begin = nullptr;
    end_t m_end = nullptr;
    find_t m_find = nullptr;

    get_iterator_key_t m_get_iterator_key = nullptr;
    get_iterator_value_t m_get_iterator_value = nullptr;
    release_iterator_t m_release_iterator = nullptr;
    advance_iterator_t m_advance_iterator = nullptr;
    copy_iterator_t m_copy_iterator = nullptr;
    iterator_equals_t m_iterator_equals = nullptr;

    insert_t m_insert = nullptr;
    replace_t m_replace = nullptr;

    erase_by_key_t m_erase_by_key = nullptr;
    erase_by_iterator_t m_erase_by_iterator = nullptr;
    clear_t m_clear = nullptr;

    const typed_access_definition * m_key_definition = nullptr;
    const wrappers::wrapper_definition * m_value_definition = nullptr;

    attributes::attribute_map m_key_metadata;
    attributes::attribute_map m_value_metadata;
  public:
    map_access_definition() noexcept;

    ~map_access_definition();

    void initialize_map_access(
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
      clear_t clear);

    const typed_access_definition & key_definition() const {
      J_ASSERT_NOT_NULL(m_key_definition);
      return *m_key_definition;
    }

    const typed_access_definition * const & key_definition_ptr() const {
      J_ASSERT_NOT_NULL(m_key_definition);
      return m_key_definition;
    }

    const wrappers::wrapper_definition & value_definition() const {
      J_ASSERT_NOT_NULL(m_value_definition);
      return *m_value_definition;
    }

    const wrappers::wrapper_definition * const & value_definition_ptr() const {
      J_ASSERT_NOT_NULL(m_value_definition);
      return m_value_definition;
    }

    void set_key_definition(const typed_access_definition * key_definition) {
      J_ASSERT(!m_key_definition, "map_access_definition::set_key_definition was called twice.");
      J_ASSERT_NOT_NULL(key_definition);
      m_key_definition = key_definition;
    }

    void set_value_definition(const wrappers::wrapper_definition * value_definition) {
      J_ASSERT(!m_value_definition,
               "map_access_definition::set_value_definition was called twice.");
      J_ASSERT_NOT_NULL(value_definition);
      m_value_definition = value_definition;
    }

    const attributes::attribute_map & value_metadata() const noexcept {
      return m_value_metadata;
    }

    attributes::attribute_map & value_metadata() noexcept {
      return m_value_metadata;
    }

    const attributes::attribute_map & key_metadata() const noexcept {
      return m_key_metadata;
    }

    attributes::attribute_map & key_metadata() noexcept {
      return m_key_metadata;
    }
  };
}
