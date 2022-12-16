#pragma once

#include "properties/access/iterator_data.hpp"
#include "properties/access/list_access_definition.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "util/ptr_like_wrapper.hpp"

namespace j::properties::access {
  inline
  namespace lists {
    class list_iterator final {
    public:
      constexpr list_iterator() noexcept = default;

      J_ALWAYS_INLINE list_iterator(const list_access_definition * def, iterator_data && data) noexcept
        : m_list_definition(def),
          m_iterator_data(data)
      {
      }

      J_INLINE_GETTER util::ptr_like_wrapper<wrappers::wrapper> operator->() const noexcept {
        return util::ptr_like_wrapper<wrappers::wrapper>{
          m_list_definition->m_get_iterator_value(m_iterator_data)
        };
      }

      J_INLINE_GETTER wrappers::wrapper operator*() const noexcept {
        return m_list_definition->m_get_iterator_value(m_iterator_data);
      }

      J_ALWAYS_INLINE list_iterator & operator++() noexcept {
        m_list_definition->m_advance_iterator(m_iterator_data);
        return *this;
      }

      J_ALWAYS_INLINE list_iterator operator++(int) noexcept {
        list_iterator result(*this);
        operator++();
        return result;
      }

      bool operator==(const list_iterator & rhs) const noexcept {
        return m_list_definition->m_iterator_equals(m_iterator_data, rhs.m_iterator_data);
      }

      J_HIDDEN J_ALWAYS_INLINE bool operator!=(const list_iterator & rhs) const noexcept {
        return !operator==(rhs);
      }

      J_HIDDEN J_ALWAYS_INLINE iterator_data & data() noexcept {
        return m_iterator_data;
      }

      J_HIDDEN J_ALWAYS_INLINE const iterator_data & data() const noexcept {
        return m_iterator_data;
      }
    private:
      const list_access_definition * m_list_definition = nullptr;
      iterator_data m_iterator_data;
    };
  }
}
