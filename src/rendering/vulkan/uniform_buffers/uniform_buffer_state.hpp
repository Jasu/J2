#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_data_source_key.hpp"
#include "rendering/vulkan/state/resource_region_variable.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class uniform_buffer_state_var final : public state::resource_region_variable<
      uniform_buffer_allocation,
      uniform_buffer_ref,
      uniform_data_source_key
    > {
    public:
      strings::string describe_precondition(
        resources::resource_wrapper * wrapper,
        uptr_t index,
        const state::precondition_instance & c
      ) const override;

      strings::string describe_postcondition(
        resources::resource_wrapper * wrapper,
        uptr_t index,
        const state::postcondition_instance & c
      ) const override;
    protected:
      uniform_buffer_ref resource_from_region(const uniform_buffer_allocation & src) const override;

      uptr_t index_from_region(const uniform_buffer_allocation & src) const override;

      uniform_buffer_allocation make_region(uniform_buffer_ref buf, uptr_t index) const override;

      bool can_transition(
        const state::transition_context & context,
        uniform_buffer_allocation source,
        uniform_data_source_key from,
        uniform_data_source_key to
      ) const override;

      state::condition_group * transition(
        state::transition_context & context,
        uniform_buffer_allocation source,
        uniform_data_source_key from,
        uniform_data_source_key to
      ) const override;
    };

    extern const uniform_buffer_state_var uniform_contents;
  }
}
