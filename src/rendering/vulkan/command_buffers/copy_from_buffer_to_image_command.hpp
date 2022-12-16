#pragma once

#include "command.hpp"
#include "rendering/vulkan/transfer/transfer_operation_source.hpp"
#include "rendering/vulkan/images/image_resource.hpp"
#include "rendering/images/image_source_key.hpp"
#include "containers/vector.hpp"
#include "geometry/rect.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class copy_from_buffer_to_image_command final : public command {
    struct image_copy {
      transfer::transfer_operation_source source;
      j::geometry::rect_u16 target_region;
    };
    images::image_ref m_target;
    mutable vector<image_copy> m_copies;
  public:
    copy_from_buffer_to_image_command(
      state::node_insertion_context & context,
      images::image_ref target,
      j::geometry::rect_u16 region,
      j::rendering::images::image_source_key image_source_key,
      const state::postcondition_initializer & postcondition
    );

    copy_from_buffer_to_image_command(
      state::node_insertion_context & context,
      images::image_ref target,
      j::rendering::images::image_source_key image_source_key,
      const state::postcondition_initializer & postcondition
    );

    bool can_merge_with(const state::node_insertion_context & context,
                        const condition_group & group) const noexcept override;

    void merge(state::node_insertion_context & context,
               condition_group && group) override;

    ~copy_from_buffer_to_image_command();

    void execute(command_context & context, state::reserved_resources_t & res) const override;

    strings::string name() const override;
  };
}
