#include "rendering/vulkan/transfer/transfer_source_buffer_set_state.hpp"
#include "rendering/vulkan/operations/copy_image_to_buffer_operation.hpp"
#include "rendering/images/image_source_manager.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::transfer {
  namespace i = j::rendering::images;

  transfer_source_buffer_ref transfer_source_image_var::resource_from_region(const transfer_operation_source & src) const {
    J_ASSERT_NOT_NULL(src);
    return src.buffer();
  }

  uptr_t transfer_source_image_var::index_from_region(const transfer_operation_source & src) const {
    J_ASSERT_NOT_NULL(src);
    return (uptr_t)src.inner();
  }

  transfer_operation_source transfer_source_image_var::make_region(
    [[maybe_unused]] transfer_source_buffer_ref buf,
    uptr_t index) const {
    J_ASSERT_NOT_NULL(buf, index);
    return transfer_operation_source((buffers::generic_suballocation*)index);
  }
  bool transfer_source_image_var::can_transition(
      const state::transition_context &,
      transfer_operation_source source,
      i::image_source_key from,
      i::image_source_key to
  ) const {
    return source && from.empty() && !to.empty();
  }

  state::condition_group * transfer_source_image_var::transition(
    state::transition_context & context,
    transfer_operation_source source,
    [[maybe_unused]] i::image_source_key from,
    i::image_source_key to
  ) const {
    J_ASSERT(can_transition(context, source, from, to), "Cannot transition image source.");
    return ::new operations::copy_image_to_buffer_operation(
      context.context,
      to,
      {
        i::image_source_manager::instance.get_info(to),
        source.target(),
      },
      {
        context.wrapper,
        index_from_region(source),
        &m_assign,
        this,
        *reinterpret_cast<uptr_t *>(&to)
      }
    );
  }

  strings::string transfer_source_image_var::describe_precondition(
    resources::resource_wrapper *, uptr_t, const state::precondition_instance & c
  ) const {
    return strings::format("TX must have {}",
                           i::image_source_manager::instance.describe(i::image_source_key(c.data)));
  }

  strings::string transfer_source_image_var::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t,
    const state::postcondition_instance & c
  ) const {
    return strings::format("TX has {}",
                           i::image_source_manager::instance.describe(i::image_source_key(c.data)));
  }

  const transfer_source_image_var transfer_source_image_content;
}
