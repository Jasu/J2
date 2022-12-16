#pragma once

#include "properties/classes/object_property_definition.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::properties::classes::object_property_getter);

namespace j::properties::classes {
  namespace detail {
    class object_iterator_pair {
    protected:
      using inner_iterator_t J_NO_DEBUG_TYPE = typename containers::uncopyable_unsorted_string_map<object_property_getter>::const_iterator;

      void * m_object;
      inner_iterator_t m_it;

      object_iterator_pair() noexcept = default;

      J_ALWAYS_INLINE object_iterator_pair(void * object, inner_iterator_t && it) noexcept
        : m_object(object),
          m_it(static_cast<inner_iterator_t &&>(it))
      {
      }
    public:
      J_INLINE_GETTER const strings::string & name() const noexcept {
        return *m_it.first;
      }

      J_INLINE_GETTER wrappers::wrapper value() {
        return m_it->second->get(m_object);
      }
    };
  }

  class object_iterator final : detail::object_iterator_pair {
    friend class object_access;
  private:
    J_ALWAYS_INLINE object_iterator(void * object, inner_iterator_t && it) noexcept
      : detail::object_iterator_pair(object, static_cast<inner_iterator_t &&>(it))
    {
    }
  public:
    object_iterator() noexcept = default;

    J_ALWAYS_INLINE object_iterator & operator++() noexcept {
      ++m_it;
      return *this;
    }

    J_ALWAYS_INLINE object_iterator operator++(int) noexcept {
      return object_iterator(m_object, m_it++);
    }

    J_INLINE_GETTER bool operator==(const object_iterator & rhs) const noexcept {
      return m_object == rhs.m_object && m_it == rhs.m_it;
    }

    J_INLINE_GETTER const detail::object_iterator_pair & operator*() const noexcept {
      return *this;
    }

    J_INLINE_GETTER detail::object_iterator_pair & operator*() noexcept {
      return *this;
    }

    J_INLINE_GETTER_NONNULL const detail::object_iterator_pair * operator->() const noexcept {
      return this;
    }

    J_INLINE_GETTER_NONNULL detail::object_iterator_pair * operator->() noexcept {
      return this;
    }
  };
}
