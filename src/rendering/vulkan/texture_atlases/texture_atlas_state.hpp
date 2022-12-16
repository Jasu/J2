#pragma once

#include "rendering/vulkan/texture_atlases/texture_atlas_resource.hpp"
#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/images/image_source_key.hpp"

namespace j::rendering::vulkan {
  inline namespace texture_atlases {
    class texture_atlas_state_variable final : public state::state_variable_definition_base {
    public:
      state::postcondition_initializer loads_image(texture_atlas_ref atlas, j::rendering::images::image_source_key image) const noexcept;
      state::precondition_initializer requires_image(texture_atlas_ref atlas, j::rendering::images::image_source_key image) const noexcept;

      strings::string describe_precondition(
        resources::resource_wrapper * wrapper,
        uptr_t detail,
        const state::precondition_instance & c
      ) const override;

      strings::string describe_postcondition(
        resources::resource_wrapper * wrapper,
        uptr_t detail,
        const state::postcondition_instance & c
      ) const override;
    protected:
      void initialize_state(resources::resource_wrapper * wrapper,
                            uptr_t detail, uptr_t & state) const noexcept override;

      bool do_can_transition(const state::transition_context & context) const noexcept override;

      state::condition_group * do_transition(state::transition_context & context) const override;
    };
    extern const texture_atlas_state_variable texture_atlas_contents;
  }
}
