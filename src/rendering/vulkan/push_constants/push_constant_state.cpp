#include "rendering/vulkan/push_constants/push_constant_state.hpp"
#include "rendering/vulkan/push_constants/push_special_constant_command.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/state/scalar_variable.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    namespace s = state;
    namespace {
      const s::equals_precondition g_constant_equals;
      const s::postcondition_definition g_constant_push;
    }

    s::precondition_initializer push_constant_state_assigner::operator==(special_push_constant value) const noexcept {
      return s::precondition_initializer(
        nullptr,
        m_value + (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        &g_constant_equals,
        &push_constant,
        (uptr_t)value);
    }

    s::postcondition_initializer push_constant_state_assigner::operator=(special_push_constant value) const noexcept {
      return s::postcondition_initializer(
        nullptr,
        m_value + (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        &g_constant_push,
        &push_constant,
        (uptr_t)value);
    }

    s::precondition_initializer push_constant_state_assigner::operator==(const void * value) const noexcept {
      return s::precondition_initializer(
        nullptr,
        m_value + (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        &g_constant_equals,
        &push_constant,
        (uptr_t)value);
    }

    s::postcondition_initializer push_constant_state_assigner::operator=(const void * value) const noexcept {
      return s::postcondition_initializer(
        nullptr,
        m_value + (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        &g_constant_push,
        &push_constant,
        (uptr_t)value);
    }

    s::postcondition_initializer push_constant_state_assigner::clear() const noexcept {
      return s::postcondition_initializer(
        nullptr,
        m_value + (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        &g_constant_push,
        &push_constant,
        UPTR_MAX);
    }

    void push_constant_state_variable::initialize_state(resources::resource_wrapper *, uptr_t, uptr_t & state) const noexcept {
      state = UPTR_MAX;
    }

    strings::string push_constant_state_variable::describe_precondition(
      resources::resource_wrapper *,
      uptr_t index,
      const state::precondition_instance & c
    ) const {
      auto idx = index - (uptr_t)command_buffers::command_buffer_state::push_constant_start;
      switch (c.data) {
      case (uptr_t)special_push_constant::viewport_size_vec2:
        return strings::format("Const #{} must be viewport size (vec2)", idx);
      case (uptr_t)special_push_constant::inverse_viewport_size_vec2:
        return strings::format("Const #{} must be inv. viewport size (vec2)", idx);
      case UPTR_MAX:
        return strings::format("Const #{} must be clear", idx);
      default:
        return strings::format("Const #{} == 0x{:x}", idx, c.data);
      }
    }

    strings::string push_constant_state_variable::describe_postcondition(
      resources::resource_wrapper *,
      uptr_t index,
      const state::postcondition_instance & c
    ) const {
      auto idx = index - (uptr_t)command_buffers::command_buffer_state::push_constant_start;
      switch (c.data) {
      case (uptr_t)special_push_constant::viewport_size_vec2:
        return strings::format("Push viewport size (vec2) to #{}", idx);
      case (uptr_t)special_push_constant::inverse_viewport_size_vec2:
        return strings::format("Push inv. viewport size (vec2) to #{}", idx);
      case UPTR_MAX:
        return strings::format("Clear constant #{}", idx);
      default:
        return strings::format("Push constant #{} = 0x{:x}", idx, c.data);
      }
    }

    bool push_constant_state_variable::do_can_transition(const state::transition_context & context) const {
      return context.to.data < (uptr_t)special_push_constant::max;
    }

    state::condition_group * push_constant_state_variable::do_transition(state::transition_context & context) const {
      J_ASSERT(do_can_transition(context), "Cannot transition.");
      return ::new push_special_constant_command(
        context.context,
        context.index - (uptr_t)command_buffers::command_buffer_state::push_constant_start,
        special_push_constant(context.to.data));
    }

    const push_constant_state_variable push_constant;
  }
}
