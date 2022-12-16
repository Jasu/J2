#pragma once

#include "hzd/concepts.hpp"

namespace j::mem {
  template<typename T>
  inline void deleter(void * J_NOT_NULL_NOESCAPE t J_VAR_ALIGNED(8)) noexcept {
    ::delete reinterpret_cast<T*>(t);
  }

  template<typename T>
  inline void destructor(void * J_NOT_NULL_NOESCAPE target J_VAR_ALIGNED(8)) noexcept {
    reinterpret_cast<T*>(target)->~T();
  }

  inline void null_deleter(void * J_NOT_NULL_NOESCAPE J_VAR_ALIGNED(8)) noexcept {
  }

  using deleter_t = void (*)(void * J_NOT_NULL_NOESCAPE J_VAR_ALIGNED(8)) noexcept;

  template<typename T>
  J_NO_DEBUG constexpr inline deleter_t deleter_v = &deleter<T>;

  template<TriviallyDestructible T>
  J_NO_DEBUG constexpr inline deleter_t deleter_v<T> = nullptr;

  template<typename T>
  J_NO_DEBUG constexpr inline deleter_t destructor_v = &destructor<T>;

  template<TriviallyDestructible T>
  J_NO_DEBUG constexpr inline deleter_t destructor_v<T> = nullptr;
}
