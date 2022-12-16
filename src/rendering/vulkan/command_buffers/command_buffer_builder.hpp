#pragma once

#include "rendering/vulkan/state/state_tracker.hpp"
#include "rendering/vulkan/images/image_resource.hpp"
#include "rendering/vulkan/rendering/indirect_buffer_resource.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"
#include "rendering/images/image_source_key.hpp"
#include "geometry/rect.hpp"
#include "draw_indirect_count_command.hpp"
#include "rendering/vulkan/state/node_insertion_context.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  class command;

  struct command_buffer_builder {
    const render_context & context;
    state::state_tracker * m_state_tracker;

    command_buffer_builder(const render_context & context, state::state_tracker * J_NOT_NULL tracker) noexcept;

    ~command_buffer_builder();

    command_buffer_builder(const command_buffer_builder &) = delete;
    command_buffer_builder & operator=(const command_buffer_builder &) = delete;
    command_buffer_builder(command_buffer_builder &&) = delete;
    command_buffer_builder & operator=(command_buffer_builder &&) = delete;

    command_buffer_builder & begin_render_pass();

    command_buffer_builder & end_render_pass();

    command_buffer_builder & push_viewport_size(u8_t push_constant_index);

    command_buffer_builder & push_inverse_viewport_size(u8_t push_constant_index);

    command_buffer_builder & push_constant(
      u8_t push_constant_index,
      const void * J_NOT_NULL data,
      sz_t size);

    template<typename T>
    J_ALWAYS_INLINE command_buffer_builder & push_constant(u8_t push_constant_index, const T & data) {
      return push_constant(push_constant_index, &data, sizeof(T));
    }

    template<u32_t N>
    command_buffer_builder & draw_indirect_count(
      rendering::indirect_count_buffer_ref buf, const state::condition_instance (& conditions)[N])
    {
      return create_command<draw_indirect_count_command>(
        static_cast<rendering::indirect_count_buffer_ref &&>(buf), conditions);
    }

    command_buffer_builder & bind_pipeline(rendering::pipeline_ref pipe);

    command_buffer_builder & set_viewport(const j::geometry::rect_u16 & viewport);

    command_buffer_builder & set_image_contents(
      images::image_ref image,
      j::rendering::images::image_source_key source
    );

    const state::state_tracker & state_tracker() const noexcept {
      return *m_state_tracker;
    }

    state::state_tracker & state_tracker() noexcept {
      return *m_state_tracker;
    }

    state::node_insertion_context insertion_context() noexcept {
      return {
        *m_state_tracker,
        context,
      };
    }

    command_buffer_builder & add_command(state::condition_group * J_NOT_NULL cmd) {
      m_state_tracker->add_node(context, cmd);
      return *this;
    }

    template<typename Cmd, typename... Args>
    command_buffer_builder & create_command(Args && ... args) {
      state::node_insertion_context c{
        *m_state_tracker,
        context,
      };
      c.begin_add_node();
      Cmd * cmd = ::new Cmd(c, static_cast<Args &&>(args)...);
      c.finish_add_node(cmd);
      m_state_tracker->add_node(context, cmd);
      return *this;
    }
  };
}
