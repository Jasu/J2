#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"


namespace j::util {
  namespace detail {
    template<typename T, u8_t Index>
    class J_HIDDEN tagged_union_base {
    protected:
      J_ALWAYS_INLINE static void maybe_destroy(void * ptr, u8_t index) noexcept;

      J_ALWAYS_INLINE static u8_t initialize(void * ptr, T && t) noexcept;

      J_ALWAYS_INLINE static u8_t initialize(void * ptr, const T & t);

      J_ALWAYS_INLINE static void maybe_copy_initialize(void * ptr, u8_t index, const void * src);

      J_ALWAYS_INLINE static void maybe_move_initialize(void * ptr, u8_t index, void * src) noexcept;

      J_ALWAYS_INLINE constexpr static u8_t index(T *) noexcept {
        return Index;
      }
    };

    template<typename T, u8_t Index>
    J_ALWAYS_INLINE void tagged_union_base<T, Index>::maybe_destroy(void * ptr, u8_t index) noexcept {
      if (index == Index) {
        reinterpret_cast<T*>(ptr)->~T();
      }
    }

    template<typename T, u8_t Index>
    J_ALWAYS_INLINE u8_t tagged_union_base<T, Index>::initialize(void * ptr, T && t) noexcept {
      ::new (ptr) T(static_cast<T &&>(t));
      return Index;
    }

    template<typename T, u8_t Index>
    J_ALWAYS_INLINE u8_t tagged_union_base<T, Index>::initialize(void * ptr, const T & t) {
      ::new (ptr) T(t);
      return Index;
    }

    template<typename T, u8_t Index>
    J_ALWAYS_INLINE void tagged_union_base<T, Index>::maybe_copy_initialize(void * ptr, u8_t index, const void * src) {
      if (index == Index) {
        tagged_union_base::initialize(ptr, *reinterpret_cast<const T*>(src));
      }
    }

    template<typename T, u8_t Index>
    J_ALWAYS_INLINE void tagged_union_base<T, Index>::maybe_move_initialize(void * ptr, u8_t index, void * src) noexcept {
      if (index == Index) {
        tagged_union_base::initialize(ptr, static_cast<T &&>(*reinterpret_cast<T*>(src)));
      }
    }

    template<sz_t Size, sz_t Align, typename... Base>
    class tagged_union final : private Base... {
      using Base::initialize...;
      using Base::index...;

      alignas(Align) char m_data[Size];
      u8_t m_index = 0;

      void destroy() noexcept;

      template<typename T> struct disable_if_self { using type = bool; };
      template<> struct disable_if_self<tagged_union &> { };
      template<> struct disable_if_self<const tagged_union &> { };

    public:
      tagged_union() noexcept = default;
      tagged_union(const tagged_union & rhs);
      tagged_union & operator=(const tagged_union & rhs);
      tagged_union(tagged_union && rhs) noexcept;
      tagged_union & operator=(tagged_union && rhs) noexcept;
      ~tagged_union();

      template<typename T>
      J_HIDDEN explicit tagged_union(T && rhs) noexcept(j::is_rref_v<T &&>);

      template<typename T>
      J_HIDDEN tagged_union & operator=(T && rhs) noexcept(j::is_rref_v<T &&>);

      template<typename T>
      J_ALWAYS_INLINE J_HIDDEN bool is() const noexcept;

      template<typename T>
      J_ALWAYS_INLINE J_HIDDEN T & as();

      template<typename T>
      J_ALWAYS_INLINE J_HIDDEN const T & as() const;

      J_ALWAYS_INLINE bool empty() const noexcept {
        return !m_index;
      }

      J_ALWAYS_INLINE bool operator!() const noexcept {
        return !m_index;
      }
      J_ALWAYS_INLINE explicit operator bool() const noexcept {
        return m_index;
      }
    };

    template<sz_t Size, sz_t Align, typename... Base>
    void tagged_union<Size, Align, Base...>::destroy() noexcept {
      (Base::maybe_destroy(&m_data, m_index), ...);
      m_index = 0;
    }

    template<sz_t Size, sz_t Align, typename... Base>
    tagged_union<Size, Align, Base...>::~tagged_union() {
      destroy();
    }

    template<sz_t Size, sz_t Align, typename... Base>
    tagged_union<Size, Align, Base...>::tagged_union(tagged_union && rhs) noexcept
      : m_index(rhs.m_index)
    {
      (Base::maybe_move_initialize(m_data, m_index, rhs.m_data), ...);
    }

    template<sz_t Size, sz_t Align, typename... Base>
    tagged_union<Size, Align, Base...> & tagged_union<Size, Align, Base...>::operator=(tagged_union && rhs) noexcept {
      if (&rhs != this) {
        destroy();
        m_index = rhs.m_index;
        (Base::maybe_move_initialize(m_data, m_index, rhs.m_data), ...);
      }
      return *this;
    }

    template<sz_t Size, sz_t Align, typename... Base>
    tagged_union<Size, Align, Base...>::tagged_union(const tagged_union & rhs)
      : m_index(rhs.m_index)
    {
      (Base::maybe_copy_initialize(m_data, m_index, rhs.m_data), ...);
    }

    template<sz_t Size, sz_t Align, typename... Base>
    tagged_union<Size, Align, Base...> & tagged_union<Size, Align, Base...>::operator=(const tagged_union & rhs) {
      if (&rhs != this) {
        destroy();
        m_index = rhs.m_index;
        (Base::maybe_copy_initialize(m_data, m_index, rhs.m_data), ...);
      }
      return *this;
    }

    template<sz_t Size, sz_t Align, typename... Base>
    template<typename T>
    J_HIDDEN tagged_union<Size, Align, Base...>::tagged_union(T && rhs) noexcept(j::is_rref_v<T &&>)
      : m_index(initialize(m_data, static_cast<T &&>(rhs)))
    {
    }

    template<sz_t Size, sz_t Align, typename... Base>
    template<typename T>
    J_HIDDEN tagged_union<Size, Align, Base...> & tagged_union<Size, Align, Base...>::operator=(T && rhs)
      noexcept(j::is_rref_v<T &&>)
    {
      destroy();
      m_index = initialize(m_data, static_cast<T &&>(rhs));
      return *this;
    }

    template<sz_t Size, sz_t Align, typename... Base>
    template<typename T>
    J_HIDDEN J_ALWAYS_INLINE bool tagged_union<Size, Align, Base...>::is() const noexcept {
      return m_index == index(static_cast<T*>(nullptr));
    }

    template<sz_t Size, sz_t Align, typename... Base>
    template<typename T>
    J_ALWAYS_INLINE J_HIDDEN T & tagged_union<Size, Align, Base...>::as() {
      J_ASSERT(index(static_cast<T*>(nullptr)) == m_index,
               "Tagged union does not contain the correct type.");
      return *reinterpret_cast<T*>(m_data);
    }

    template<sz_t Size, sz_t Align, typename... Base>
    template<typename T>
    J_ALWAYS_INLINE J_HIDDEN const T & tagged_union<Size, Align, Base...>::as() const {
      return const_cast<tagged_union*>(this)->as<T>();
    }

    template<sz_t Size, sz_t Align, typename Bases, typename... Types>
    struct J_TYPE_HIDDEN tagged_union_helper;

    template<sz_t Size, sz_t Align, typename... Bases, typename Head, typename... Tail>
    struct J_TYPE_HIDDEN tagged_union_helper<Size, Align, void (Bases...), Head, Tail...>
      : tagged_union_helper<
          (Size > sizeof(Head) ? Size : sizeof(Head)),
          (Align > alignof(Head) ? Align : alignof(Head)),
          void (Bases..., tagged_union_base<Head, static_cast<u8_t>(sizeof...(Bases) + 1)>),
          Tail...
        >
    { };

    template<sz_t Size, sz_t Align, typename... Bases>
    struct J_TYPE_HIDDEN tagged_union_helper<Size, Align, void (Bases...)> {
      using type J_NO_DEBUG_TYPE = tagged_union<Size, Align, Bases...>;
    };
  }


  template<typename... Types>
  using tagged_union J_NO_DEBUG_TYPE = typename detail::tagged_union_helper<0, 0, void(), Types...>::type;
}
