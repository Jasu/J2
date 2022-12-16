#include "properties/access/set_access_definition.hpp"
#include "properties/access/set_access_iterator.hpp"

namespace j::properties::access {
  set_access_definition::set_access_definition() noexcept {
  }

  set_access_definition::~set_access_definition() {
  }

  void set_access_definition::initialize_set_access(
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
  ) noexcept {
    m_get_size = get_size;
    m_empty = empty;
    m_contains = contains;

    m_begin = begin;
    m_end = end;
    m_find = find;

    m_get_iterator_item = get_iterator_item;
    m_release_iterator = release_iterator;
    m_advance_iterator = advance_iterator;
    m_copy_iterator = copy_iterator;
    m_iterator_equals = iterator_equals;

    m_insert = insert;

    m_erase_by_item = erase_by_item;
    m_erase_by_iterator = erase_by_iterator;
    m_clear = clear;

    // release iterator may be empty
    J_ASSERT_NOT_NULL(m_get_size, m_empty, m_contains, m_begin, m_end);
    J_ASSERT_NOT_NULL(m_find, m_get_iterator_item, m_advance_iterator, m_copy_iterator, m_iterator_equals);
    J_ASSERT_NOT_NULL(m_insert, m_clear, m_erase_by_item, m_erase_by_iterator);
  }

  const wrappers::wrapper_definition & set_access_definition::item_definition() const {
    J_ASSERT_NOT_NULL(m_item_definition);
    return *m_item_definition;
  }

  void set_access_definition::set_item_definition(const wrappers::wrapper_definition * item_definition) {
    J_ASSERT(!m_item_definition,
             "set_access_definition::set_item_definition was called twice.");
    J_ASSERT_NOT_NULL(item_definition);
    m_item_definition = item_definition;
  }


  set_iterator::set_iterator(const set_iterator & rhs)
    : m_def(rhs.m_def),
      m_data(rhs.m_def ? rhs.m_def->m_copy_iterator(rhs.m_data) : iterator_data())
  {
  }

  set_iterator & set_iterator::operator=(const set_iterator & rhs) {
    if (this != &rhs) {
      if (m_def && m_def->m_release_iterator) {
        m_def->m_release_iterator(m_data);
      }
      m_def = rhs.m_def;
      if (m_def) {
        m_data = m_def->m_copy_iterator(rhs.m_data);
      }
    }
    return *this;
  }

  set_iterator & set_iterator::operator=(set_iterator && rhs) noexcept {
    if (this != &rhs) {
      if (m_def && m_def->m_release_iterator) {
        m_def->m_release_iterator(m_data);
      }
      m_def = rhs.m_def;
      m_data = static_cast<iterator_data &&>(rhs.m_data);
      rhs.m_def = nullptr;
    }
    return *this;
  }
}
