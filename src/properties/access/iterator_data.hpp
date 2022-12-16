#pragma once

#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"

namespace j::properties::access {
  class iterator_data {
    uptr_t m_data[2] = { 0 };
    template<typename T> struct J_TYPE_HIDDEN disable_if_self { using type = void; };
    template<> struct J_TYPE_HIDDEN disable_if_self<iterator_data &> { };
    template<> struct J_TYPE_HIDDEN disable_if_self<const iterator_data &> { };
  public:
    constexpr iterator_data() noexcept = default;

    template<typename Data, typename = typename disable_if_self<Data &>::type>
    J_ALWAYS_INLINE explicit iterator_data(Data && data) noexcept {
      ::new (m_data) decltype(identity(data)) (static_cast<Data &&>(data));
    }

    template<typename Data>
    J_INLINE_GETTER Data & as() noexcept {
      return *reinterpret_cast<Data*>(m_data);
    }

    template<typename Data>
    J_INLINE_GETTER const Data & as() const noexcept {
      return *reinterpret_cast<const Data*>(m_data);
    }
  };
}
