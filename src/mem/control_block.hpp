#pragma once

#include "hzd/utility.hpp"

namespace j::mem::detail {
  struct shared_ptr_control_block;

  struct shared_ptr_control_block final {
    u32_t num_references = 0U;
    u32_t num_weak_references = 0U;
    void * target = nullptr;
    void (*destructor)(void* J_NONNULL_NOESCAPE) noexcept;

    void remove_last_reference() noexcept;

    void remove_reference() noexcept {
      // `if (!--num_references)` won't do - the reference must stay alive
      // until after the deleter is called, to ensure that weak references
      // keep working. Thus, some logic is in remove_last_reference instead.
      if (num_references > 1) {
        --num_references;
      } else {
        remove_last_reference();
      }
    }
  };
}
