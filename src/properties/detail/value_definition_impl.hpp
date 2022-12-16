#pragma once

#include "properties/detail/value_definition.hpp"
#include "hzd/integer_traits.hpp"
#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"

namespace j::properties::detail {
  constexpr value_definition::value_definition(
      delete_t deleter,
      delete_t destruct,
      copy_construct_t copy_construct,
      copy_construct_t move_construct,
      copy_construct_in_place_t copy_construct_in_place,
      copy_construct_in_place_t move_construct_in_place,
      copy_assign_t copy_assign,
      copy_assign_t move_assign,
      sz_t in_place_size
    ) noexcept
    : m_delete(deleter),
      m_destruct(destruct),
      m_copy_construct(copy_construct),
      m_move_construct(move_construct ? move_construct : copy_construct),
      m_copy_construct_in_place(copy_construct_in_place),
      m_move_construct_in_place(move_construct_in_place
                                ? move_construct_in_place
                                : copy_construct_in_place),
      m_copy_assign(copy_assign),
      m_move_assign(move_assign ? move_assign : copy_assign),
      m_in_place_size(in_place_size)
  {
  }

  namespace detail {
    template<bool IsScalar, typename Type>
    struct J_HIDDEN value_traits {
      J_NO_DEBUG inline constexpr static bool is_destructible = is_destructible_v<Type>;
      J_NO_DEBUG inline constexpr static bool is_copy_constructible = is_copy_constructible_v<Type>;
      J_NO_DEBUG inline constexpr static bool is_move_constructible = is_move_constructible_v<Type>;
      J_NO_DEBUG inline constexpr static bool is_copy_assignable = is_copy_assignable_v<Type>;
      J_NO_DEBUG inline constexpr static bool is_move_assignable = is_move_assignable_v<Type>;
    };

    template<typename Type>
    struct J_HIDDEN value_traits<true, Type> {
      J_NO_DEBUG inline constexpr static bool is_destructible = true;
      J_NO_DEBUG inline constexpr static bool is_copy_constructible = true;
      J_NO_DEBUG inline constexpr static bool is_move_constructible = true;
      J_NO_DEBUG inline constexpr static bool is_copy_assignable = true;
      J_NO_DEBUG inline constexpr static bool is_move_assignable = true;
    };

    template<typename Type>
    J_HIDDEN void default_deleter(void * ptr) noexcept {
      delete reinterpret_cast<Type*>(ptr);
    }

    void default_ptr_deleter(void * ptr) noexcept;

    template<typename Type>
    J_HIDDEN void default_destructor(void * ptr) noexcept {
      reinterpret_cast<Type*>(ptr)->~Type();
    }

    void default_ptr_destructor(void * ptr) noexcept;

    template<typename Type, typename Access, bool IsDestructible, auto Fn = &Access::deleter>
    J_INLINE_GETTER constexpr value_definition::delete_t get_deleter(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsDestructible>
    J_INLINE_GETTER value_definition::delete_t get_deleter(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_deleter;
      } else if constexpr (IsDestructible) {
        return &default_deleter<Type>;
      } else {
        return nullptr;
      }
    }


    template<typename Type, typename Access, bool IsDestructible, auto Fn = &Access::destructor>
    J_INLINE_GETTER constexpr value_definition::delete_t get_destructor(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsDestructible>
    J_INLINE_GETTER constexpr value_definition::delete_t get_destructor(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_destructor;
      } else if constexpr (IsDestructible) {
        if constexpr (j::is_trivially_destructible_v<Type>) {
          return &default_ptr_destructor;
        } else {
          return &default_destructor<Type>;
        }
      } else {
        return nullptr;
      }
    }

    void * default_ptr_copy_construct(void * ptr);

    template<typename Type>
    J_HIDDEN void * default_copy_construct(void * ptr) {
      return ::new Type(*reinterpret_cast<const Type *>(ptr));
    }

    void default_ptr_copy_construct_in_place(void * to, void * ptr) noexcept;

    template<typename Type>
    J_HIDDEN void default_copy_construct_in_place(void * to, void * ptr) {
      ::new (to) Type(*reinterpret_cast<const Type *>(ptr));
    }


    template<typename Type, typename Access, bool IsCopyConstructible, auto Fn = &Access::copy_construct>
    J_INLINE_GETTER constexpr value_definition::copy_construct_t get_copy_constructor(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsCopyConstructible>
    J_INLINE_GETTER constexpr value_definition::copy_construct_t get_copy_constructor(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_construct;
      } else if constexpr (IsCopyConstructible) {
        return &default_copy_construct<Type>;
      } else {
        return nullptr;
      }
    }

    template<typename Type, typename Access, bool IsCopyConstructible, auto Fn = &Access::copy_construct_in_place>
    J_INLINE_GETTER constexpr value_definition::copy_construct_in_place_t get_in_place_copy_constructor(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsCopyConstructible>
    J_INLINE_GETTER constexpr value_definition::copy_construct_in_place_t get_in_place_copy_constructor(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_construct_in_place;
      } else if constexpr (IsCopyConstructible) {
        return &default_copy_construct_in_place<Type>;
      } else {
        return nullptr;
      }
    }

    template<typename Type>
    J_HIDDEN void * default_move_construct(void * ptr) {
      return ::new Type(static_cast<Type &&>(*reinterpret_cast<Type *>(ptr)));
    }

    template<typename Type>
    J_HIDDEN void default_move_construct_in_place(void * to, void * ptr) {
      ::new (to) Type(static_cast<Type &&>(*reinterpret_cast<Type *>(ptr)));
    }

    template<typename Type, typename Access, bool IsMoveConstructible, auto Fn = &Access::move_construct>
    J_INLINE_GETTER constexpr value_definition::copy_construct_t get_access_move_constructor(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsMoveConstructible>
    J_INLINE_GETTER constexpr value_definition::copy_construct_t get_move_constructor(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_construct;
      } else if constexpr (IsMoveConstructible) {
        return &default_move_construct<Type>;
      } else {
        return nullptr;
      }
    }

    template<typename Type, typename Access, bool IsMoveConstructible, auto Fn = &Access::move_construct_in_place>
    J_INLINE_GETTER constexpr value_definition::copy_construct_in_place_t get_in_place_move_constructor(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsMoveConstructible>
    J_INLINE_GETTER constexpr value_definition::copy_construct_in_place_t get_in_place_move_constructor(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_construct_in_place;
      } else if constexpr (IsMoveConstructible) {
        return &default_move_construct_in_place<Type>;
      } else {
        return nullptr;
      }
    }

    void default_ptr_copy_assign(void * to, void * from);

    template<typename Type>
    J_HIDDEN void default_copy_assign(void * to, void * from) {
      *reinterpret_cast<Type*>(to) = *reinterpret_cast<const Type*>(from);
    }

    template<typename Type, typename Access, bool, auto Fn = &Access::copy_assign>
    J_INLINE_GETTER constexpr auto get_copy_assign(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsCopyAssignable>
    J_INLINE_GETTER constexpr auto get_copy_assign(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_assign;
      } else if constexpr (IsCopyAssignable) {
        return &default_copy_assign<Type>;
      } else {
        return nullptr;
      }
    }

    template<typename Type>
    J_HIDDEN void default_move_assign(void * to, void * from) {
      *reinterpret_cast<Type*>(to) = static_cast<Type &&>(*reinterpret_cast<Type*>(from));
    }

    template<typename Type, typename Access, bool, auto Fn = &Access::move_assign>
    J_INLINE_GETTER constexpr auto get_move_assign(int) noexcept {
      return Fn;
    }

    template<typename Type, typename Access, bool IsMoveAssignable>
    J_INLINE_GETTER constexpr auto get_move_assign(long) noexcept {
      if constexpr (is_ptr_v<Type>) {
        return &default_ptr_copy_assign;
      } else if constexpr (IsMoveAssignable) {
        return &default_move_assign<Type>;
      } else {
        return nullptr;
      }
    }

    template<typename Type, bool IsScalar>
    J_INLINE_GETTER constexpr sz_t get_in_place_size(long) noexcept {
      return SZ_MAX;
    }

    template<typename Type, bool IsScalar, sz_t Sz = sizeof(Type)>
    J_INLINE_GETTER constexpr sz_t get_in_place_size(int) noexcept {
      if constexpr (IsScalar) {
        return Sz;
      } else if constexpr (Sz > sizeof(void*)) {
        return SZ_MAX;
      } else if constexpr (is_nicely_copyable_v<Type>) {
        return Sz;
      } else {
        return SZ_MAX;
      }
    }
  }

  template<typename Type>
  J_INLINE_GETTER constexpr bool is_scalar() noexcept {
    if constexpr (is_ptr_v<Type>) {
      return true;
    } else if constexpr (is_arithmetic_v<Type>) {
      return true;
    } else if constexpr (is_enum_v<Type>) {
      return true;
    } else {
      return false;
    }
  }

  template<typename Type, typename Access = void>
  J_HIDDEN constexpr value_definition make_value_definition() noexcept {
    constexpr bool is_scalar_v= is_scalar<Type>();
    using t J_NO_DEBUG_TYPE = detail::value_traits<is_scalar_v, Type>;
    return {
      detail::get_deleter<Type, Access, t::is_destructible>(0),
      detail::get_destructor<Type, Access, t::is_destructible>(0),
      detail::get_copy_constructor<Type, Access, t::is_copy_constructible>(0),
      detail::get_move_constructor<Type, Access, t::is_move_constructible>(0),
      detail::get_in_place_copy_constructor<Type, Access, t::is_copy_constructible>(0),
      detail::get_in_place_move_constructor<Type, Access, t::is_move_constructible>(0),
      detail::get_copy_assign<Type, Access, t::is_copy_assignable>(0),
      detail::get_move_assign<Type, Access, t::is_move_assignable>(0),
      detail::get_in_place_size<Type, is_scalar_v>(0)
    };
  }
}
