#include "rendering/vulkan/uniform_buffers/uniform_buffer_state.hpp"

#include "rendering/vulkan/uniform_buffers/copy_to_uniform_buffer_operation.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    strings::string uniform_buffer_state_var::describe_precondition(
      resources::resource_wrapper *, uptr_t, const state::precondition_instance &) const
    { return "Uniform must contain"; }

    strings::string uniform_buffer_state_var::describe_postcondition(
      resources::resource_wrapper *, uptr_t, const state::postcondition_instance &) const
    { return "Uniform contains"; }

    uniform_buffer_ref uniform_buffer_state_var::resource_from_region(
      const uniform_buffer_allocation & src) const
    {
      J_ASSERT_NOT_NULL(src);
      return src.buffer;
    }

    uptr_t uniform_buffer_state_var::index_from_region(
      const uniform_buffer_allocation & src) const
    {
      J_ASSERT_NOT_NULL(src);
      return ((uptr_t)src.offset << 32ULL)
        | ((uptr_t)src.allocation_index << 16ULL)
        | ((uptr_t)src.layout_index);
    }

    uniform_buffer_allocation uniform_buffer_state_var::make_region(uniform_buffer_ref buf, uptr_t index) const {
      J_ASSERT_NOT_NULL(buf);
      return uniform_buffer_allocation(
        static_cast<uniform_buffer_ref &&>(buf),
        index >> 32U,
        index >> 16U,
        index);
    }

    bool uniform_buffer_state_var::can_transition(
      [[maybe_unused]] const state::transition_context & context,
      uniform_buffer_allocation,
      uniform_data_source_key from,
      uniform_data_source_key to) const {
      J_ASSERT_NOT_NULL(context.wrapper);
      return from != to && !to.empty();
    }

    state::condition_group * uniform_buffer_state_var::transition(
      state::transition_context & context,
      uniform_buffer_allocation source,
      [[maybe_unused]] uniform_data_source_key from,
      uniform_data_source_key to
    ) const {
      J_ASSERT(can_transition(context, source, from, to), "Cannot transition.");
      return ::new copy_to_uniform_buffer_operation(
        context.context,
        to,
        {source.buffer.get().layout(source.layout_index), source.target()},
        (*this)(source) = to);
    }

    const uniform_buffer_state_var uniform_contents;
  }
}
