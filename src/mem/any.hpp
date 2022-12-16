#pragma once

#include "type_id/type_id.hpp"
#include "hzd/type_traits.hpp"
#include "mem/deleter.hpp"

namespace j::strings {
  class string;
  template<typename> class basic_string_view;
  using string_view = basic_string_view<char>;
  using const_string_view = basic_string_view<const char>;
}
namespace j::mem {
  class any;

  namespace detail {
    [[noreturn]] void throw_invalid_type();
  }

  class any final {
  public:
    template<typename T>
    static constexpr mem::any create(type_identity_t<T> && value) {
      return mem::any((type_tag<T> *)nullptr, static_cast<T &&>(value));
    }

    J_A(AI,HIDDEN,ND) inline constexpr any() noexcept = default;

    template<typename T>
    J_ALWAYS_INLINE explicit any(T && value) {
      using decayed_t J_NO_DEBUG_TYPE = j::decay_t<T>;
      if constexpr (is_lref_v<T>) {
        initialize<decayed_t>(const_cast<const decayed_t &>(value));
      } else {
        initialize<decayed_t>(static_cast<T &&>(value));
      }
    }

    J_A(AI,HIDDEN,ND) inline constexpr any(any && rhs) noexcept
      : m_value(rhs.m_value),
        m_deleter(rhs.m_deleter),
        m_type(rhs.m_type)
    {
      rhs.m_value = nullptr;
      rhs.m_deleter = nullptr;
      rhs.m_type.reset();
    }

    template<typename T>
    J_ALWAYS_INLINE any & operator=(T && rhs) {
      reset();
      using decayed_t J_NO_DEBUG_TYPE = j::decay_t<T>;
      if constexpr (is_lref_v<T>) {
        initialize<decayed_t>(const_cast<const decayed_t &>(rhs));
      } else {
        initialize<decayed_t>(static_cast<T &&>(rhs));
      }
      return *this;
    }

    any & operator=(any && rhs) noexcept;

    constexpr ~any() {
      if (m_type.get_tag_bit(0)) {
        m_deleter(m_value);
      }
    }

    constexpr void reset() noexcept {
      if (m_type.get_tag_bit(0)) {
        m_deleter(m_value);
        m_deleter = nullptr;
      }
      m_value = nullptr;
      m_type.reset();
    }

    template<typename T>
    J_INLINE_GETTER T & get() {
      if (J_UNLIKELY(!is<T>())) { detail::throw_invalid_type(); }
      return *reinterpret_cast<T*>(as_void_star());
    }

    template<typename T>
    J_INLINE_GETTER const T & get() const
    { return const_cast<any*>(this)->get<T>(); }

    template<typename T>
    J_INLINE_GETTER T & get_unsafe() noexcept
     {return *reinterpret_cast<T*>(as_void_star()); }

    template<typename T>
    J_INLINE_GETTER const T & get_unsafe() const noexcept
    { return *reinterpret_cast<const T*>(as_void_star()); }

    template<typename T>
    J_INLINE_GETTER bool is() const noexcept {
      // Compare "the hard way", to avoid operator== lookup, which seems to be slow
      // for type_infos.
      return m_type.name() == typeid(T).name();
    }

    J_INLINE_GETTER void * as_void_star() noexcept J_RETURNS_ALIGNED(16) {
      return m_type.get_tag_bit(0)
        ? m_value
        : reinterpret_cast<void*>(&m_value);
    }

    template<typename T>
    J_INLINE_GETTER T * maybe_get() noexcept J_RETURNS_ALIGNED(16)
    { return is<T>() ? reinterpret_cast<T*>(as_void_star()) : nullptr; }

    template<typename T>
    J_INLINE_GETTER const T * maybe_get() const noexcept J_RETURNS_ALIGNED(16) {
      return is<T>() ? reinterpret_cast<const T*>(as_void_star()) : nullptr;
    }

    J_INLINE_GETTER const void * as_void_star() const noexcept J_RETURNS_ALIGNED(16)
    { return const_cast<any*>(this)->as_void_star(); }

    J_INLINE_GETTER type_id::type_id type() const noexcept
    { return m_type; }

    J_A(ND,HIDDEN) any(const any &) = delete;
    J_A(ND,HIDDEN) any & operator=(const any &) = delete;
    J_A(ND,HIDDEN) any(any &) = delete;
    J_A(ND,HIDDEN) any & operator=(any &) = delete;
  private:
    template<typename T>
    J_ALWAYS_INLINE explicit any(type_tag<T> *, T && value) {
      initialize<T, T &&>(static_cast<T &&>(value));
    }

    void * m_value = nullptr;
    deleter_t m_deleter = nullptr;
    type_id::type_id m_type;

    template<typename T, typename A>
    void initialize(A && arg) {
      if constexpr (j::is_trivially_copyable_v<T> && sizeof(T) <= 16U) {
        m_type = type_id::type_id(typeid(T));
        ::new (&m_value) T(static_cast<A &&>(arg));
      } else {
        m_type = type_id::type_id(typeid(T), 1);
        m_deleter = &deleter<T>;
        m_value = ::new T(static_cast<A &&>(arg));
      }
   }
  };



  extern template void any::initialize<strings::const_string_view, const strings::const_string_view &>(const strings::const_string_view &);
  extern template void any::initialize<strings::const_string_view, strings::const_string_view>(strings::const_string_view &&);

  extern template void any::initialize<strings::string, const strings::string &>(const strings::string &);
  extern template void any::initialize<strings::string, strings::string>(strings::string &&);
}
