#pragma once

#include "properties/access/set_access_definition.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/access/iterator_data.hpp"
#include "util/ptr_like_wrapper.hpp"

namespace j::properties::access {
  class set_iterator {
    friend class set_access;
  protected:
    const set_access_definition * m_def = nullptr;
    iterator_data m_data;

  public:
    constexpr set_iterator() noexcept = default;

    iterator_data & data() noexcept {
      return m_data;
    }

    const iterator_data & data() const noexcept {
      return m_data;
    }

    set_iterator(const set_access_definition * def, iterator_data && data) noexcept
      : m_def(def),
        m_data(static_cast<iterator_data &&>(data))
    {
    }

    set_iterator(const set_iterator & rhs);

    set_iterator(set_iterator && rhs) noexcept
      : m_def(rhs.m_def),
        m_data(static_cast<iterator_data &&>(rhs.m_data))
    {
      rhs.m_def = nullptr;
    }

    set_iterator & operator=(const set_iterator & rhs);

    set_iterator & operator=(set_iterator && rhs) noexcept;

    util::ptr_like_wrapper<wrappers::wrapper> operator->() {
      return util::ptr_like_wrapper<wrappers::wrapper>{m_def->m_get_iterator_item(m_data)};
    }

    wrappers::wrapper operator*() {
      return m_def->m_get_iterator_item(m_data);
    }

    set_iterator & operator++() {
      m_def->m_advance_iterator(m_data);
      return *this;
    }

    set_iterator operator++(int) {
      set_iterator result(*this);
      ++*this;
      return result;
    }

    bool operator==(const set_iterator & rhs) const noexcept {
      return m_def == rhs.m_def &&
        (!m_def || m_def->m_iterator_equals(m_data, rhs.m_data));
    }

    bool operator!=(const set_iterator & rhs) const noexcept {
      return !operator==(rhs);
    }

    ~set_iterator() {
      if (m_def && m_def->m_release_iterator) {
        m_def->m_release_iterator(m_data);
      }
    }
  };
}
