#pragma once

#include "properties/access/map_access_definition.hpp"
#include "properties/access/typed_access.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/access/iterator_data.hpp"

namespace j::properties::access {
  class map_iterator_pair {
    friend class map_access;
  protected:
    const map_access_definition * m_def = nullptr;
    iterator_data m_data;

  public:
    constexpr map_iterator_pair() noexcept = default;

    iterator_data & data() noexcept;

    const iterator_data & data() const noexcept;

    map_iterator_pair(const map_access_definition * def, iterator_data && data) noexcept;

    map_iterator_pair(const map_iterator_pair & rhs);

    map_iterator_pair(map_iterator_pair && rhs) noexcept;

    map_iterator_pair & operator=(const map_iterator_pair & rhs);

    map_iterator_pair & operator=(map_iterator_pair && rhs) noexcept;

    ~map_iterator_pair();

    typed_access key();

    wrappers::wrapper value();
  };

  inline iterator_data & map_iterator_pair::data() noexcept {
      return m_data;
    }

  inline const iterator_data & map_iterator_pair::data() const noexcept {
      return m_data;
    }

  inline map_iterator_pair::map_iterator_pair(const map_access_definition * def, iterator_data && data) noexcept
      : m_def(def),
        m_data(static_cast<iterator_data &&>(data))
    {
    }

  inline map_iterator_pair::map_iterator_pair(const map_iterator_pair & rhs)
      : m_def(rhs.m_def),
        m_data(rhs.m_def ? rhs.m_def->m_copy_iterator(rhs.m_data) : iterator_data())
    {
    }

  inline map_iterator_pair::map_iterator_pair(map_iterator_pair && rhs) noexcept
      : m_def(rhs.m_def),
        m_data(static_cast<iterator_data &&>(rhs.m_data))
    {
      rhs.m_def = nullptr;
    }

  inline map_iterator_pair & map_iterator_pair::operator=(const map_iterator_pair & rhs) {
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

  inline map_iterator_pair & map_iterator_pair::operator=(map_iterator_pair && rhs) noexcept {
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

  inline map_iterator_pair::~map_iterator_pair() {
      if (m_def && m_def->m_release_iterator) {
        m_def->m_release_iterator(m_data);
      }
    }

  inline typed_access map_iterator_pair::key() {
      return m_def->m_get_iterator_key(m_data);
    }

  inline wrappers::wrapper map_iterator_pair::value() {
      return m_def->m_get_iterator_value(m_data);
    }

  class map_iterator final : private map_iterator_pair {
    friend class map_access;
  public:
    map_iterator() = default;

    map_iterator(const map_access_definition * def, iterator_data && data) noexcept;

    map_iterator_pair * operator->() noexcept;

    map_iterator_pair & operator*() noexcept;

    map_iterator & operator++();

    map_iterator operator++(int);

    inline bool operator==(const map_iterator & rhs) const noexcept;
  };

  inline map_iterator::map_iterator(const map_access_definition * def, iterator_data && data) noexcept
    : map_iterator_pair(def, static_cast<iterator_data &&>(data))
  {
  }

  inline map_iterator_pair * map_iterator::operator->() noexcept {
    return static_cast<map_iterator_pair*>(this);
  }

  inline map_iterator_pair & map_iterator::operator*() noexcept {
    return *static_cast<map_iterator_pair*>(this);
  }

  inline map_iterator & map_iterator::operator++() {
    m_def->m_advance_iterator(m_data);
    return *this;
  }

  inline map_iterator map_iterator::operator++(int) {
    map_iterator result(*this);
    operator++();
    return result;
  }

  inline bool map_iterator::operator==(const map_iterator & rhs) const noexcept {
    return m_def == rhs.m_def &&
      (!m_def || m_def->m_iterator_equals(m_data, rhs.m_data));
  }
}
