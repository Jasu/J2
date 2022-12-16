#include "command_buffer_state.hpp"
#include "bind_pipeline_command.hpp"
#include "end_render_pass_command.hpp"
#include "begin_render_pass_command.hpp"
#include "rendering/vulkan/rendering/render_pass_resource.hpp"
#include "strings/format.hpp"
#include "set_viewport_command.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/context/render_context.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  bool bound_pipeline_var::can_transition(
    const state::transition_context &,
    resources::resource_wrapper * from,
    resources::resource_wrapper * to
  ) const {
    return to && to != from;
  }

  state::condition_group * bound_pipeline_var::transition(
    state::transition_context & c,
    [[maybe_unused]] resources::resource_wrapper * from,
    resources::resource_wrapper * to
  ) const {
    J_ASSUME_NOT_NULL(to);
    J_ASSERT(from != to, "Invalid transition.");
    return ::new bind_pipeline_command(c.context, rendering::pipeline_ref(to));
  }

  static const char * get_pipeline_name(uptr_t data) noexcept {
    if (!data) { return "NULL"; }
    auto w = reinterpret_cast<const resources::resource_wrapper *>(data);
    return (w->status == resources::wrapper_status::initialized)
      ? reinterpret_cast<const rendering::pipeline *>(w->data)->name : nullptr;
  }

  strings::string bound_pipeline_var::describe_precondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::precondition_instance & c
  ) const {
    if (&c.definition() == &m_equals) {
      auto n = get_pipeline_name(c.data);
      return n ? strings::format("{} must be bound", n) : "Pipeline must be bound";
    } else {
      return "Unknown pipeline precondition";
    }
  }

  strings::string bound_pipeline_var::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::postcondition_instance & c
  ) const {
    if (&c.definition() == &m_assign) {
      auto n = get_pipeline_name(c.data);
      return n ? strings::format("Bind {}", n) : "Bind pipeline";
    } else {
      return "Unknown pipeline postcondition";
    }
  }

  strings::string viewport_var::describe_precondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::precondition_instance &
  ) const {
    return "Viewport equals";
  }

  strings::string viewport_var::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::postcondition_instance &
  ) const {
    return "Set viewport";
  }

  bool viewport_var::can_transition(
      const state::transition_context &,
      geometry::rect_u16 from,
      geometry::rect_u16 to
  ) const {
    return from != to;
  }

  state::condition_group * viewport_var::transition(
      state::transition_context & c,
      [[maybe_unused]] geometry::rect_u16 from,
      geometry::rect_u16 to
  ) const {
    J_ASSERT(can_transition(c, from, to), "Cannot transition viewport.");
    return ::new set_viewport_command(c.context, to);
  }

  bool is_in_render_pass_var::can_transition(
    const state::transition_context &,
    bool from,
    bool to
  ) const {
    return from != to;
  }

  state::condition_group * is_in_render_pass_var::transition(
    state::transition_context & c,
    bool from,
    [[maybe_unused]] bool to
  ) const {
    J_ASSERT(from != to, "No transition.");
    return from ? (state::condition_group*)::new end_render_pass_command(c.context)
      : ::new begin_render_pass_command(c.context);
  }

  strings::string is_in_render_pass_var::describe_precondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::precondition_instance & c
  ) const {
    if (&c.definition() == &m_equals) {
      return c.data ? "In render pass" : "Out of render pass";
    } else {
      return "Unknown render pass precondition";
    }
  }

  strings::string is_in_render_pass_var::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::postcondition_instance & c
  ) const {
    if (&c.definition() == &m_assign) {
      return c.data ? "Enter render pass" : "Exit render pass";
    } else {
      return "Unknown render pass postcondition";
    }
  }

  const bound_pipeline_var bound_pipeline{command_buffer_state::pipeline};
  const viewport_var viewport{command_buffer_state::viewport};
  const is_in_render_pass_var is_in_render_pass{command_buffer_state::render_pass};
}
