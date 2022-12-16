#include "rendering/vulkan/operations/update_descriptor_sets_operation.hpp"
#include "exceptions/assert_lite.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/images/image.hpp"
#include "rendering/vulkan/images/image_view.hpp"
#include "rendering/vulkan/samplers/sampler.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"
#include "rendering/vulkan/descriptors/descriptor_set.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_description.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "strings/string.hpp"
#include "containers/trivial_array.hpp"

#include <vulkan/vulkan.h>

namespace j::rendering::vulkan::operations {
  update_descriptor_sets_operation::update_descriptor_sets_operation(
    state::node_insertion_context & context,
    descriptors::descriptor_set_ref descriptor_set,
    bindings_t && bindings
  ) : m_descriptor_set(static_cast<descriptors::descriptor_set_ref &&>(descriptor_set)),
      m_bindings(static_cast<bindings_t &&>(bindings))
  {
    this->is_bound_to_epoch(command_buffers::epochs::before_render_pass);
    J_ASSERT_NOT_NULL(m_descriptor_set);
    J_ASSERT(m_bindings.size() > 0, "Bindings cannot be empty.");
    u32_t mask = 0;
    for (auto & b : m_bindings) {
      J_ASSERT(!(mask & (1U << b.binding())), "Double-bind.");
      mask |= 1U << b.binding();
      J_ASSERT(!b.state().empty(), "Tried to make an empty binding.");
      context.add_postcondition(descriptors::descriptor_binding(m_descriptor_set, b.binding()) = b.state());
    }
  }

  update_descriptor_sets_operation::update_descriptor_sets_operation(
    state::node_insertion_context & context,
    descriptors::descriptor_set_ref descriptor_set,
    u32_t index,
    descriptors::binding_state state)
  : m_descriptor_set(static_cast<descriptors::descriptor_set_ref &&>(descriptor_set)),
    m_bindings(containers::uninitialized, 1)
  {
    this->is_bound_to_epoch(command_buffers::epochs::before_render_pass);
    J_ASSERT_NOT_NULL(m_descriptor_set);
    m_bindings.initialize_element(index, state);
    context.add_postcondition(descriptors::descriptor_binding(m_descriptor_set, index) = state);
  }

  void update_descriptor_sets_operation::execute(command_context & context, state::reserved_resources_t &) const {
    u32_t num_uniforms = 0;
    u32_t num_images = 0;
    for (auto & b : m_bindings) {
      if (b.state().is_image()) {
        ++num_images;
      } else {
        J_ASSERT(b.state().is_uniform() || b.state().is_dynamic_uniform(), "Binding is empty.");
        ++num_uniforms;
      }
    }
    VkDescriptorBufferInfo buffer_info[num_uniforms];
    VkDescriptorImageInfo image_info[num_images];
    VkWriteDescriptorSet write_info[num_uniforms + num_images];
    u32_t image_index = 0, buffer_index = 0, write_index = 0;
    auto & ds = context.render_context->get(m_descriptor_set);
    auto & l = context.render_context->get(ds.layout);
    for (auto & b : m_bindings) {
      write_info[write_index] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ds.vk_descriptor_set,
        .dstBinding = b.binding(),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
      };
      auto & binding = l.find_binding(b.binding());
      write_info[write_index].descriptorType = binding.type;
      switch (binding.type) {
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
        J_ASSERT(b.state().is_image(), "Invalid type");
        images::image_view_ref img_view = context.render_context->get(b.state().as_image()).default_image_view_ref();
        image_info[image_index] = {
          .sampler = context.render_context->default_sampler().vk_sampler,
          .imageView = context.render_context->get(img_view).vk_image_view,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        write_info[write_index].pImageInfo = &image_info[image_index++];
        break;
      }
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        J_ASSERT(b.state().is_uniform(), "Invalid type");
        uniform_buffers::uniform_buffer & buf = context.render_context->get(b.state().as_uniform_buffer());
        const auto index = b.state().get_uniform_buffer_allocation_index();
        J_ASSERT(buf.allocation_size(index) == (u32_t)binding.uniform_layout.size, "Size mismatch");
        buffer_info[buffer_index] = {
          .buffer = buf.vk_buffer,
          .offset = buf.allocation_offset(index),
          .range = buf.allocation_size(index),
        };
        write_info[write_index].pBufferInfo = &buffer_info[buffer_index++];
        break;
      }
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
        J_ASSERT(b.state().is_dynamic_uniform(), "Invalid type");
        uniform_buffers::uniform_buffer & buf = context.render_context->get(b.state().as_uniform_buffer());
        buffer_info[buffer_index] = {
          .buffer = buf.vk_buffer,
          .offset = 0,
          .range = (VkDeviceSize)binding.uniform_layout.size,
        };
        write_info[write_index].pBufferInfo = &buffer_info[buffer_index++];
        break;
      }
      default:
        J_THROW("Unknown binding type.");
      }
      write_index++;
    }
    vkUpdateDescriptorSets(
      context.render_context->device().vk_device,
      write_index,
      write_info,
      0,
      nullptr);
  }

  strings::string update_descriptor_sets_operation::name() const {
    return "Update descriptor set";
  }

  strings::string update_descriptor_sets_operation::debug_details() const {
    strings::string result;
    for (auto & b : m_bindings) {
      if (b.state().is_uniform()) {
        result += "Uniform ";
      } else if (b.state().is_image()) {
        result += "Image ";
      }
    }
    return result;
  }

  update_descriptor_sets_operation::~update_descriptor_sets_operation() {
  }
}
