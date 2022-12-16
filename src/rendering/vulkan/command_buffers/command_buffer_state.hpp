#pragma once

#include "rendering/vulkan/state/scalar_variable.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/state/epochs.hpp"
#include "geometry/rect.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  namespace epochs {
    inline constexpr state::epoch_definition render_pass
      { 0U, 29U, 3U, "Render pass"};
    inline constexpr state::epoch_definition pipeline_binding
      { 1U, 22U, 7U, "Pipeline binding"};
    inline constexpr state::epoch_definition draw_command
      { 2U, 11U, 11U, "Draw command"};

    inline constexpr state::epoch_key before_render_pass{0U, 0U};
    inline constexpr state::epoch_key in_render_pass{0U, 1U};
    inline constexpr state::epoch_key after_render_pass{0U, 2U};
  }
  enum class command_buffer_state : u32_t {
    pipeline,
    render_pass,
    viewport,

    descriptor_sets_start = 64U,
    descriptor_sets_end = 80U,

    push_constant_start = 128U,
    push_constant_end = 256U,

    vertex_bindings_start = push_constant_end,
    vertex_bindings_end = (u32_t)vertex_bindings_start + 32U,
  };

  class bound_pipeline_var final
    : public state::global_scalar_variable<rendering::pipeline_ref> {
  public:
    using state::global_scalar_variable<rendering::pipeline_ref>::global_scalar_variable;

    using state::global_scalar_variable<rendering::pipeline_ref>::operator==;
    using state::global_scalar_variable<rendering::pipeline_ref>::operator=;

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
    bool can_transition(
      const state::transition_context &,
      resources::resource_wrapper * from,
      resources::resource_wrapper * to
    ) const override;

    state::condition_group * transition(
      state::transition_context &,
      resources::resource_wrapper * from,
      resources::resource_wrapper * to
    ) const override;
  };

  class viewport_var final : public state::global_scalar_variable<j::geometry::rect_u16> {
  public:
    using state::global_scalar_variable<j::geometry::rect_u16>::global_scalar_variable;
    using state::global_scalar_variable<j::geometry::rect_u16>::operator==;
    using state::global_scalar_variable<j::geometry::rect_u16>::operator=;

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
    bool can_transition(
      const state::transition_context &,
      j::geometry::rect_u16 from,
      j::geometry::rect_u16 to
    ) const override;

    state::condition_group * transition(
      state::transition_context &,
      j::geometry::rect_u16 from,
      j::geometry::rect_u16 to
    ) const override;
  };

  class is_in_render_pass_var final : public state::global_scalar_variable<bool> {
  public:
    using state::global_scalar_variable<bool>::global_scalar_variable;

    using state::global_scalar_variable<bool>::operator==;
    using state::global_scalar_variable<bool>::operator=;

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
    bool can_transition(
      const state::transition_context & context,
      bool from,
      bool to
    ) const override;

    state::condition_group * transition(
      state::transition_context & context,
      bool from,
      bool to
    ) const override final;
  };


  extern const bound_pipeline_var bound_pipeline;
  extern const viewport_var viewport;
  extern const is_in_render_pass_var is_in_render_pass;
}
