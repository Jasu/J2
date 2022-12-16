#pragma once

#include "rendering/vulkan/transfer/transfer_source_buffer.hpp"
#include "rendering/vulkan/state/resource_region_variable.hpp"
#include "rendering/vulkan/transfer/transfer_operation_source.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_resource.hpp"
#include "rendering/images/image_source_key.hpp"

namespace j::rendering::vulkan::transfer {
  class transfer_source_image_var final : public state::resource_region_variable<
    transfer_operation_source,
    transfer_source_buffer_ref,
    j::rendering::images::image_source_key,
    state::noninvalidating_flag
  >
  {
  public:
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
    transfer_source_buffer_ref resource_from_region(const transfer_operation_source & src) const override;

    uptr_t index_from_region(const transfer_operation_source & src) const override;

    transfer_operation_source make_region(transfer_source_buffer_ref buf, uptr_t index) const override;

    bool can_transition(
      const state::transition_context & context,
      transfer_operation_source source,
      j::rendering::images::image_source_key from,
      j::rendering::images::image_source_key to
    ) const override;

    state::condition_group * transition(
      state::transition_context & context,
      transfer_operation_source ource,
      j::rendering::images::image_source_key from,
      j::rendering::images::image_source_key to
    ) const override;
  };

  extern const transfer_source_image_var transfer_source_image_content;
}
