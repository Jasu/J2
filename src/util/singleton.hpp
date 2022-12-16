#pragma once

#include "hzd/type_traits.hpp"

namespace j::util {
  template<typename Class>
  struct alignas(Class) singleton final {
    J_A(NI) void initialize() noexcept(is_nothrow_default_constructible_v<Class>) {
      is_initialized = true;
      ::new (instance) Class;
    }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline Class * unsafe_instance() noexcept {
      return reinterpret_cast<Class*>(instance);
    }

    [[nodiscard]] inline Class * get() noexcept(is_nothrow_default_constructible_v<Class>) {
      if (J_UNLIKELY(!is_initialized)) {
        initialize();
      }
      return reinterpret_cast<Class*>(instance);
    }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline Class * operator->() noexcept(is_nothrow_default_constructible_v<Class>) {
      return get();
    }

    J_A(AI,ND,NE,NODISC,HIDDEN) inline Class & operator*() noexcept(is_nothrow_default_constructible_v<Class>) {
      return *get();
    }

    J_A(AI,ND,NE,HIDDEN) inline constexpr singleton() noexcept = default;

    singleton(const singleton &) = delete;

    J_A(AI,NE) inline ~singleton() {
      if (is_initialized) {
        reinterpret_cast<Class*>(instance)->~Class();
        is_initialized = 0;
      }
    }

    bool is_initialized = false;
    char instance[sizeof(Class)] = { 0 };
  };
}
