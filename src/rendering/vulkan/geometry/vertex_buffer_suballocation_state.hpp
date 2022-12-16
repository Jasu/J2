#pragma once

#include "rendering/vulkan/state/resource_region_variable.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_suballocation.hpp"
#include "rendering/vertex_data/vertex_data_source_key.hpp"

namespace j::rendering::vulkan::geometry {
  class vertex_buffer_suballocation_var final : public state::resource_region_variable<
    vertex_buffer_suballocation,
    vertex_buffer_ref,
    vertex_data::vertex_data_source_key
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
    vertex_buffer_ref resource_from_region(const vertex_buffer_suballocation & src) const override;

    uptr_t index_from_region(const vertex_buffer_suballocation & src) const override;

    vertex_buffer_suballocation make_region(vertex_buffer_ref buf, uptr_t index) const override;

    bool can_transition(
      const state::transition_context & context,
      vertex_buffer_suballocation source,
      vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to
    ) const override;

    state::condition_group * transition(
      state::transition_context & context,
      vertex_buffer_suballocation source,
      vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to
    ) const override;

    void initialize_state(resources::resource_wrapper * buffer, uptr_t index, uptr_t & state) const noexcept override;

    void commit(state::state_context & context) const override;
  };

  extern const vertex_buffer_suballocation_var vertex_suballocation_contents;
}
