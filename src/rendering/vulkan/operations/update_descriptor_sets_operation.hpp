#pragma once

#include "rendering/vulkan/operations/operation.hpp"

#include "containers/trivial_array_fwd.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"

namespace j::rendering::vulkan::operations {
  class update_descriptor_sets_operation final : public operation {
  public:
    using bindings_t = trivial_array<descriptors::keyed_binding_state>;

    /// Construct an update operation for multiple bindings.
    update_descriptor_sets_operation(state::node_insertion_context & context,
                                     descriptors::descriptor_set_ref descriptor_set,
                                     bindings_t && bindings);

    /// Construct an update operation for a single binding.
    ///
    /// \note This is used to transition descriptor sets automatically.
    update_descriptor_sets_operation(state::node_insertion_context & context,
                                     descriptors::descriptor_set_ref descriptor_set,
                                     u32_t index,
                                     descriptors::binding_state state);


    void execute(command_context & context, state::reserved_resources_t &) const override;

    strings::string name() const override;

    strings::string debug_details() const override;

    ~update_descriptor_sets_operation();
  private:
    descriptors::descriptor_set_ref m_descriptor_set;
    bindings_t m_bindings;
  };
}
