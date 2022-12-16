#pragma once

#include "properties/access/typed_access.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::wrappers {
  class wrapper;
}

namespace j::properties::access {
  inline namespace lists {
    class list_iterator;
  }
  class iterator_data;

  class list_access_definition final : public typed_access_definition {
    friend class list_access;
    friend class lists::list_iterator;
  public:
    using get_sz_t J_NO_DEBUG_TYPE = sz_t (*)(const void*) noexcept;
    using empty_t J_NO_DEBUG_TYPE = bool (*)(const void*) noexcept;
    using at_t J_NO_DEBUG_TYPE = wrappers::wrapper (*)(void *, sz_t i);

    using get_iterator_t J_NO_DEBUG_TYPE = list_iterator (*)(void * list);

    using erase_by_iterator_t J_NO_DEBUG_TYPE = list_iterator (*)(void * list, const list_iterator &);
    using erase_by_index_t J_NO_DEBUG_TYPE = void (*)(void * list, sz_t);

    using clear_t J_NO_DEBUG_TYPE = void (*)(void *);

    using push_back_t J_NO_DEBUG_TYPE = void (*)(void * list, const typed_access & value);
    using insert_t J_NO_DEBUG_TYPE = list_iterator (*)(void * list, const list_iterator &, const typed_access & value);

    using advance_iterator_t J_NO_DEBUG_TYPE = void (*)(iterator_data & it) noexcept;
    using iterator_equals_t J_NO_DEBUG_TYPE = bool (*)(const iterator_data & lhs, const iterator_data & rhs) noexcept;
    using get_iterator_value_t J_NO_DEBUG_TYPE = wrappers::wrapper (*)(const iterator_data & lhs) noexcept;

  private:
    get_sz_t m_get_size = nullptr;
    empty_t m_empty = nullptr;
    at_t m_at = nullptr;

    get_iterator_t m_begin = nullptr;
    get_iterator_t m_end = nullptr;

    push_back_t m_push_back = nullptr;
    insert_t m_insert = nullptr;

    erase_by_iterator_t m_erase_by_iterator = nullptr;
    erase_by_index_t m_erase_by_index = nullptr;
    clear_t m_clear = nullptr;

    advance_iterator_t m_advance_iterator = nullptr;
    iterator_equals_t m_iterator_equals = nullptr;
    get_iterator_value_t m_get_iterator_value = nullptr;

    const wrappers::wrapper_definition * m_item_definition = nullptr;
    attributes::attribute_map m_item_metadata;
  public:
    list_access_definition() noexcept;

    ~list_access_definition();

    void initialize_list_access(
      get_sz_t get_size,
      empty_t empty,
      at_t at,

      get_iterator_t begin,
      get_iterator_t end,

      push_back_t push_back,
      insert_t insert,

      erase_by_iterator_t erase_by_iterator,
      erase_by_index_t erase_by_index,
      clear_t clear,

      advance_iterator_t advance_iterator,
      iterator_equals_t iterator_equals,
      get_iterator_value_t get_iterator_value);

    const wrappers::wrapper_definition * item_definition() const {
      J_ASSERT_NOT_NULL(m_item_definition);
      return m_item_definition;
    }

    void set_item_definition(const wrappers::wrapper_definition * item_definition);

    attributes::attribute_map & item_metadata() noexcept {
      return m_item_metadata;
    }

    const attributes::attribute_map & item_metadata() const noexcept {
      return m_item_metadata;
    }
  };
}
