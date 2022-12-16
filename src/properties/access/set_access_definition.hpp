#pragma once

#include "properties/access/typed_access.hpp"
#include "attributes/attribute_map.hpp"
#include "containers/pair.hpp"

namespace j::properties::wrappers {
  class wrapper_definition;
  class wrapper;
}

namespace j::properties::access {
  class typed_access_definition;
  class iterator_data;

  class set_iterator;

  class set_access_definition final : public typed_access_definition {
    friend class set_access;
    friend class set_iterator;
  public:
    using get_sz_t = sz_t (*)(const void * set) noexcept;
    using empty_t = bool (*)(const void * set) noexcept;
    using contains_t = bool (*)(void * set, const typed_access & key);
    using clear_t = void (*)(void * set);

    using begin_t = iterator_data (*) (void * set);
    using end_t = iterator_data (*) (void * set);
    using find_t = iterator_data (*) (void * set, const typed_access & item);

    using erase_by_item_t = sz_t (*) (void * set, const typed_access & item);
    using erase_by_iterator_t = iterator_data (*) (void * set, const iterator_data & it);

    using get_iterator_item_t = wrappers::wrapper (*)(iterator_data & it);
    using release_iterator_t = void (*)(iterator_data & it) noexcept;
    using advance_iterator_t = void (*)(iterator_data & it);
    using copy_iterator_t = iterator_data (*)(const iterator_data & it);
    using iterator_equals_t = bool (*)(const iterator_data & lhs, const iterator_data & rhs);

    using insert_t = pair<iterator_data, bool> (*)(void * set, const typed_access & item);
  private:
    get_sz_t m_get_size = nullptr;
    empty_t m_empty = nullptr;
    contains_t m_contains = nullptr;

    begin_t m_begin = nullptr;
    end_t m_end = nullptr;
    find_t m_find = nullptr;

    get_iterator_item_t m_get_iterator_item = nullptr;
    release_iterator_t m_release_iterator = nullptr;
    advance_iterator_t m_advance_iterator = nullptr;
    copy_iterator_t m_copy_iterator = nullptr;
    iterator_equals_t m_iterator_equals = nullptr;

    insert_t m_insert = nullptr;

    erase_by_item_t m_erase_by_item = nullptr;
    erase_by_iterator_t m_erase_by_iterator = nullptr;
    clear_t m_clear = nullptr;

    const wrappers::wrapper_definition * m_item_definition = nullptr;

    attributes::attribute_map m_item_metadata;
  public:
    set_access_definition() noexcept;

    void initialize_set_access(
      get_sz_t get_size,
      empty_t empty,
      contains_t contains,

      begin_t begin,
      end_t end,
      find_t find,

      get_iterator_item_t get_iterator_item,
      release_iterator_t release_iterator,
      advance_iterator_t advance_iterator,
      copy_iterator_t copy_iterator,
      iterator_equals_t iterator_equals,

      insert_t insert,

      erase_by_item_t erase_by_item,
      erase_by_iterator_t erase_by_iterator,
      clear_t clear
    ) noexcept;

    ~set_access_definition();

    const wrappers::wrapper_definition & item_definition() const;

    void set_item_definition(const wrappers::wrapper_definition * item_definition);

    const attributes::attribute_map & item_metadata() const noexcept {
      return m_item_metadata;
    }

    attributes::attribute_map & item_metadata() noexcept {
      return m_item_metadata;
    }
  };
}
