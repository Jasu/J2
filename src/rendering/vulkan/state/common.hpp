#pragma once

#include "exceptions/assert.hpp"

namespace j::rendering::vulkan::resources {
  struct resource_wrapper;
}

#define J_ASSERT_STATE_IDX_POINTS_TO_VARIABLE(I) J_ASSERT(::j::rendering::vulkan::state::points_to_variable(I), \
                                                "State index {:x} is not a variable.", (I))
#define J_ASSERT_STATE_IDX_POINTS_TO_VAR_TAIL(I) J_ASSERT(::j::rendering::vulkan::state::points_to_variable_tail(I), \
                                                "State index {:x} is not variable tail.", (I))
#define J_ASSERT_STATE_IDX_POINTS_TO_VAR_HEAD(I) J_ASSERT(::j::rendering::vulkan::state::points_to_variable_head(I), \
                                                "State index {:x} is not variable head.", (I))
#define J_ASSERT_STATE_IDX_POINTS_TO_CONDITION(I) J_ASSERT(::j::rendering::vulkan::state::points_to_condition(I), \
                                                 "State index {:x} is not a condition.")

namespace j::rendering::vulkan::state {
  inline constexpr u32_t variable_head_mask_v = 0x4000U;
  inline constexpr u32_t variable_tail_mask_v = 0x8000U;
  inline constexpr u32_t variable_mask_v = 0xC000U;
  inline constexpr u32_t index_mask_v = 0x3FFFU;

  J_INLINE_GETTER constexpr bool points_to_variable_head(u32_t idx) noexcept
  { return idx & variable_head_mask_v; }

  J_INLINE_GETTER constexpr bool points_to_variable_tail(u32_t idx) noexcept
  { return idx & variable_tail_mask_v; }

  J_INLINE_GETTER constexpr bool points_to_variable(u32_t idx) noexcept {
    const u32_t flags = idx & variable_mask_v;
    return flags == variable_head_mask_v || flags == variable_tail_mask_v;
  }

  J_INLINE_GETTER constexpr bool points_to_condition(u32_t idx) noexcept
  { return !(idx & variable_mask_v); }

  J_INLINE_GETTER constexpr u32_t get_index(u32_t idx) noexcept
  { return idx & index_mask_v; }

  struct state_variable_key final {
    /// Index of the state variable in the resource wrapper.
    const uptr_t index = 0U;
    /// Resource wrapper of the state variable.
    resources::resource_wrapper * const wrapper = nullptr;

    J_INLINE_GETTER constexpr bool operator==(const state_variable_key & rhs) const noexcept = default;
  };
}
