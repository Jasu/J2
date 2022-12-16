#pragma once

#include "mem/weak_ptr.hpp"

namespace j::mem {
  template<typename T>
  struct enable_shared_from_this {
    J_ALWAYS_INLINE mem::shared_ptr<T> shared_from_this() {
      return __weak_this.lock();
    }

    J_ALWAYS_INLINE mem::shared_ptr<const T> shared_from_this() const {
      return __weak_this.lock();
    }

    mem::weak_ptr<T> __weak_this;
  };
}
