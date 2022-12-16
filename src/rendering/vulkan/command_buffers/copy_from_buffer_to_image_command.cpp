#include "copy_from_buffer_to_image_command.hpp"

#include "command_buffer_state.hpp"
#include "command_context.hpp"
#include "rendering/vulkan/images/image.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/rendering/reserved_resource.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_set.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_set_state.hpp"
#include "rendering/images/image_buffer_info.hpp"
#include "rendering/images/image_source_manager.hpp"
#include "containers/obstack.hpp"
#include "containers/vector.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  namespace i = j::rendering::images;

  copy_from_buffer_to_image_command::copy_from_buffer_to_image_command(
    state::node_insertion_context & context,
    images::image_ref target,
    geometry::rect_u16 region,
    i::image_source_key image_source_key,
    const state::postcondition_initializer & postcondition
  ) : m_target(static_cast<images::image_ref &&>(target)) {
    if (region.width() == U16_MAX) {
      auto & img = context.render_context.get(target);
      region.set_width(img.size.x);
      region.set_height(img.size.y);
    }
    J_ASSERT_NOT_NULL(m_target, image_source_key, region);
    const auto info = i::image_source_manager::instance.get_info(image_source_key);

    this->insertion_options = state::merges_with_siblings_flag | state::merges_with_parent_flag;

    auto source = context.render_context.transfer_source_buffer_set().allocate(context.render_context, info.size_bytes());
    J_ASSERT(source, "Allocation failed.");

    this->is_bound_to_epoch(epochs::before_render_pass);
    context.add_precondition(images::image_layout(m_target) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.add_precondition(transfer::transfer_source_image_content(source) == image_source_key);
    context.add_postcondition(postcondition);
    m_copies.push_back({
        static_cast<transfer::transfer_operation_source &&>(source),
        region,
    });
  }

  copy_from_buffer_to_image_command::copy_from_buffer_to_image_command(
    state::node_insertion_context & context,
    images::image_ref target,
    i::image_source_key image_source_key,
    const state::postcondition_initializer & postcondition
  ) : copy_from_buffer_to_image_command(
      context,
      target,
      {0U, 0U, U16_MAX, U16_MAX },
      image_source_key,
      postcondition)
  {
  }

  copy_from_buffer_to_image_command::~copy_from_buffer_to_image_command()
  {
  }

  bool copy_from_buffer_to_image_command::can_merge_with(const state::node_insertion_context &,
                                                         const condition_group & group) const noexcept {
    auto & g = static_cast<const copy_from_buffer_to_image_command &>(group);
    J_ASSERT(&g != this, "Cannot merge with self.");
    return g.m_target == m_target && g.m_copies[0].source.buffer() == m_copies[0].source.buffer();
  }

  void copy_from_buffer_to_image_command::merge(state::node_insertion_context & context,
                                                condition_group && group) {
    J_ASSERT(&group != this, "Cannot merge with self.");
    auto & g = static_cast<copy_from_buffer_to_image_command &>(group);
    J_ASSERT(g.m_target == m_target, "Cannot merge copies to different images.");
    m_copies.reserve(m_copies.size() + g.m_copies.size());
    for (u32_t i = 0; i < num_preconditions; ++i) {
      context.move_condition(*this, i);
    }
    for (u32_t i = 1; i < g.num_preconditions; ++i) {
      context.move_condition(g, i);
    }
    for (u32_t i = num_preconditions; i < size(); ++i) {
      context.move_condition(*this, i);
    }
    for (u32_t i = g.num_preconditions; i < g.size(); ++i) {
      context.move_condition(g, i);
    }
    for (auto & c : g.m_copies) {
      m_copies.emplace_back(static_cast<image_copy &&>(c));
    }
  }

  void copy_from_buffer_to_image_command::execute(command_context & context,
                                                  state::reserved_resources_t & res) const
  {
    VkBufferImageCopy copies[m_copies.size()];
    for (u32_t i = 0; i < m_copies.size(); ++i) {
      copies[i] = VkBufferImageCopy{
        .bufferOffset = m_copies[i].source.offset(),
        .imageSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = 0,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
        .imageOffset = {
          .x = m_copies[i].target_region.left(),
          .y = m_copies[i].target_region.top(),
        },
        .imageExtent = {
          .width = m_copies[i].target_region.width(),
          .height = m_copies[i].target_region.height(),
          .depth = 1,
        },
      };
      res.emplace(
        ::new transfer::transfer_operation_source(
          static_cast<transfer::transfer_operation_source &&>(m_copies[i].source)));
    }
    vkCmdCopyBufferToImage(context.vk_command_buffer,
                           context.render_context->get(m_copies[0].source.buffer()).vk_buffer,
                           context.render_context->get(m_target).vk_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           m_copies.size(),
                           copies);
  }

  strings::string copy_from_buffer_to_image_command::name() const {
    if (m_copies.size() == 1) {
      return strings::format("Blit buffer to image at {}", m_copies[0].target_region);
    } else {
      return strings::format("Blit buffer to image at {} sources", m_copies.size());
    }
  }
}
