#include "properties/access/list_access_definition.hpp"

namespace j::properties::access {
  list_access_definition::list_access_definition() noexcept {
  }

  list_access_definition::~list_access_definition() {
  }

  void list_access_definition::initialize_list_access(
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
    get_iterator_value_t get_iterator_value
  ) {
    m_get_size = get_size;
    m_empty = empty;
    m_at = at;

    m_begin = begin;
    m_end = end;

    m_push_back = push_back;
    m_insert = insert;

    m_erase_by_iterator = erase_by_iterator;
    m_erase_by_index = erase_by_index;
    m_clear = clear;

    m_advance_iterator = advance_iterator;
    m_iterator_equals = iterator_equals;
    m_get_iterator_value = get_iterator_value;

    J_ASSERT_NOT_NULL(m_get_size, m_empty, m_at, m_begin, m_end);
    J_ASSERT_NOT_NULL(m_push_back, m_insert, m_erase_by_iterator, m_erase_by_index, m_clear);
    J_ASSERT_NOT_NULL(m_advance_iterator, m_iterator_equals, m_get_iterator_value);
  }
}
