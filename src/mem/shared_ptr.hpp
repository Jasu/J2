#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::mem {
  template<typename T> struct enable_shared_from_this;
  namespace detail {
    J_A(RNN,NODISC) shared_ptr_control_block * allocate_control_block(u32_t object_size, deleter_t destructor) noexcept;

    template<typename T, typename Fn>
    struct J_TYPE_HIDDEN dynamic_deleter_record final {
      T * target;
      Fn fn;
    };

    template<typename T, typename Fn>
    J_ALWAYS_INLINE static void dynamic_deleter(void * J_NOT_NULL_NOESCAPE rec_) noexcept {
      J_NO_DEBUG const auto rec = reinterpret_cast<dynamic_deleter_record<T, Fn>*>(J_ASSUME_ALIGNED_8(rec_));
      rec->fn(rec->target);
    }

    template<typename T, auto Fn, typename = void>
    struct J_TYPE_HIDDEN wrap_deleter;

    template<typename T, typename Result, typename Param, Result (* J_NOT_NULL Fn)(Param * J_NOT_NULL), typename Enable>
    struct J_TYPE_HIDDEN wrap_deleter<T, Fn, Enable> {
      J_NO_DEBUG static void fn(void * J_NOT_NULL_NOESCAPE target) noexcept {
        Fn(static_cast<Param *>(reinterpret_cast<T*>(J_ASSUME_ALIGNED_8(target))));
      }
    };

    template<typename T, typename U, U (*Fn)(U * J_NOT_NULL_NOESCAPE) noexcept>
    struct J_TYPE_HIDDEN wrap_deleter<T, Fn, U> {
      J_NO_DEBUG static inline constexpr auto fn = Fn;
    };
  }

  template<typename Result, typename T>
  J_HIDDEN shared_ptr<Result> wrap_shared(T * J_NOT_NULL ptr, deleter_t dtor) noexcept {
    detail::shared_ptr_control_block * cb = detail::allocate_control_block(0, dtor);
    cb->target = ptr;

    if constexpr (__is_complete_type(T)) {
      if constexpr (__is_base_of(enable_shared_from_this<T>, T)) {
        cb->num_weak_references = 1;
        ptr->__weak_this = weak_ptr<T>(ptr, cb);
      }
    }
    return shared_ptr<Result>{ static_cast<Result*>(ptr), cb };
  }

  template<typename T, auto Deleter = &destructor<T>, typename V>
  [[nodiscard]] shared_ptr<T> wrap_shared(V * J_NOT_NULL ptr) noexcept {
    return wrap_shared<T>(ptr, detail::wrap_deleter<T, Deleter>::fn);
  }

  template<typename Result, typename T, typename Fn>
  J_HIDDEN shared_ptr<Result> wrap_shared(T * J_AA(NN,NOALIAS) ptr, Fn && fn) {
    using rec_t J_NO_DEBUG_TYPE = detail::dynamic_deleter_record<T, j::remove_ref_t<Fn>>;
    detail::shared_ptr_control_block * J_AA(NOALIAS) cb = detail::allocate_control_block(
      sizeof(rec_t),
      &detail::dynamic_deleter<T, j::remove_ref_t<Fn>>);
    rec_t * J_AA(NOALIAS) target = reinterpret_cast<rec_t*>(cb->target);
    ::new (target) rec_t{ptr, static_cast<Fn &&>(fn)};
    if constexpr (__is_complete_type(T)) {
      if constexpr (__is_base_of(enable_shared_from_this<T>, T)) {
        cb->num_weak_references = 1;
        ptr->__weak_this = weak_ptr<T>(ptr, cb);
      }
    }
    return shared_ptr<Result>{ static_cast<Result*>(ptr), cb };
  }

  template<typename T, auto Deleter, typename Result, typename... Args>
  shared_ptr<Result> make_shared_sized(u32_t size, Args && ... args) noexcept(is_nothrow_constructible_v<T, Args...>) {
    detail::shared_ptr_control_block * J_AA(NOALIAS)cb = detail::allocate_control_block(size, detail::wrap_deleter<T, Deleter>::fn);
    T * J_AA(NOALIAS) target = reinterpret_cast<T*>(J_ASSUME_ALIGNED_16(cb->target));
    if constexpr (is_nothrow_constructible_v<T, Args...>) {
      ::new (target) T(static_cast<Args &&>(args)...);
    } else {
      try {
        ::new (target) T(static_cast<Args &&>(args)...);
      } catch (...) {
        ::operator delete ((void*)(cb));
        throw;
      }
    }
    if constexpr (__is_base_of(enable_shared_from_this<T>, T)) {
      cb->num_weak_references = 1;
      target->__weak_this = weak_ptr<T>(target, cb);
    }
    return shared_ptr<Result>{ static_cast<Result*>(target), cb };
  }
}
