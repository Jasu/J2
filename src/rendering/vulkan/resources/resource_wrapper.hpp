#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::rendering::vulkan::resources {
  enum class wrapper_status : u8_t {
    uninitialized = 0U,
    initializing,
    initialized,
    empty,
  };

  struct resource_wrapper final {
    J_BOILERPLATE(resource_wrapper, CTOR_CE, MOVE_DEL, COPY_DEL)

    u32_t refcount = 0U;
    u32_t weak_refcount:29 = 0U;
    wrapper_status status:3 = wrapper_status::uninitialized;
    u64_t data[1];

    template<typename State>
    J_INLINE_GETTER State & resource_state() noexcept {
      constexpr auto state_size = (sizeof(State) + sizeof(u64_t) - 1) / sizeof(u64_t);
      return *reinterpret_cast<State*>(reinterpret_cast<u64_t*>(this) - state_size);
    }

    template<typename State>
    J_INLINE_GETTER const State & resource_state() const noexcept {
      constexpr auto state_size = (sizeof(State) + sizeof(u64_t) - 1) / sizeof(u64_t);
      return *reinterpret_cast<const State*>(reinterpret_cast<const u64_t*>(this) - state_size);
    }

    J_ALWAYS_INLINE void assert_not_empty() const noexcept {
      J_ASSUME(status != wrapper_status::empty);
    }
  };
}
