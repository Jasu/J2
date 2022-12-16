#pragma once

#include "rendering/vulkan/state/persistent_scalar_variable.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"
#include "rendering/vertex_data/vertex_data_source_key.hpp"

namespace j::rendering::vulkan::geometry {
  struct vertex_buffer_state {
    vertex_data::vertex_data_source_key contents;
  };

  class vertex_buffer_contents_var final
    : public state::persistent_scalar_variable<vertex_buffer_ref, &vertex_buffer_state::contents> {
  public:
    using state::persistent_scalar_variable<vertex_buffer_ref, &vertex_buffer_state::contents>::persistent_scalar_variable;

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
    bool can_transition(
      const state::transition_context & context,
      vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to) const override;

    state::condition_group * transition(
      state::transition_context & context,
      vertex_buffer_ref ref,
      vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to) const override;
  };

  extern const vertex_buffer_contents_var buffer_content;
}
