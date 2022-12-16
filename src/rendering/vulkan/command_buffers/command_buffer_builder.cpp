#include "command_buffer_builder.hpp"

#include "command_buffer_state.hpp"

#include "begin_render_pass_command.hpp"
#include "bind_descriptor_sets_command.hpp"
#include "bind_pipeline_command.hpp"
#include "copy_from_buffer_to_image_command.hpp"
#include "draw_indirect_count_command.hpp"
#include "end_render_pass_command.hpp"
#include "set_viewport_command.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/push_constants/push_constants_command.hpp"
#include "rendering/vulkan/push_constants/push_special_constant_command.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "mem/unique_ptr.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  command_buffer_builder::command_buffer_builder(const render_context & context, state::state_tracker * J_NOT_NULL tracker) noexcept
    : context(context),
      m_state_tracker(tracker)
  {
  }

  command_buffer_builder::~command_buffer_builder() {
  }

  command_buffer_builder & command_buffer_builder::set_image_contents(
    images::image_ref image,
    j::rendering::images::image_source_key source
  ) {
    J_ASSERT_NOT_NULL(image);
    create_command<copy_from_buffer_to_image_command>(
      static_cast<images::image_ref &&>(image),
      source,
      images::image_content(image) = source);
    return *this;
  }

  command_buffer_builder & command_buffer_builder::begin_render_pass() {
    create_command<begin_render_pass_command>();
    return *this;
  }

  command_buffer_builder & command_buffer_builder::end_render_pass() {
    create_command<end_render_pass_command>();
    return *this;
  }

  command_buffer_builder & command_buffer_builder::push_viewport_size(u8_t push_constant_index) {
    create_command<push_constants::push_special_constant_command>(
      push_constant_index,
      push_constants::special_push_constant::viewport_size_vec2);
    return *this;
  }

  command_buffer_builder & command_buffer_builder::push_inverse_viewport_size(u8_t push_constant_index) {
    create_command<push_constants::push_special_constant_command>(
      push_constant_index,
      push_constants::special_push_constant::inverse_viewport_size_vec2);
    return *this;
  }

  command_buffer_builder & command_buffer_builder::push_constant(u8_t push_constant_index,
                                                                 const void * J_NOT_NULL data,
                                                                 sz_t size) {
    J_ASSERT_NOT_NULL(data);
    J_ASSERT(size, "Tried to push a zero-sized push constant.");
    create_command<push_constants_command>(copy, push_constant_index, size, data, data);
    return *this;
  }

  command_buffer_builder & command_buffer_builder::bind_pipeline(rendering::pipeline_ref pipe) {
    J_ASSERT_NOT_NULL(pipe);
    context.get(pipe);
    create_command<bind_pipeline_command>(static_cast<rendering::pipeline_ref &&>(pipe));
    return *this;
  }

  command_buffer_builder & command_buffer_builder::set_viewport(const j::geometry::rect_u16 & viewport) {
    create_command<set_viewport_command>(viewport);
    return *this;
  }
}
