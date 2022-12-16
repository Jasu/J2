#include "rendering/vulkan/transfer/transfer_source_buffer_set.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_set_description.hpp"
#include "rendering/vulkan/transfer/transfer_operation_source.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "containers/trivial_array.hpp"
J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::transfer::transfer_source_buffer_ref);

namespace j::rendering::vulkan::transfer {
  transfer_source_buffer_set::transfer_source_buffer_set(const device_context_base &,
                                                         const transfer_source_buffer_set_description & desc)
    : m_buffers(containers::uninitialized, desc.count())
  {
    J_ASSERT(desc.count() > 0, "Tried to allocate zero transfer source buffers.");
    for (u32_t i = 0; i < desc.count(); ++i) {
      m_buffers.initialize_element(desc.size());
    }
  }

  transfer_source_buffer_set::~transfer_source_buffer_set() {
  }

  transfer_operation_source transfer_source_buffer_set::allocate(
    const device_context_base & context, u32_t size)
  {
    J_ASSERT(size > 0, "Tried to allocate zero bytes.");
    for (u8_t i = 0; i < m_buffers.size(); ++i) {
      if (auto result = context.get(m_buffers[i]).allocate(size)) {
        return result;
      }
    }
    return transfer_operation_source();
  }
  DEFINE_RESOURCE_DEFINITION(transfer_source_buffer_set, transfer_source_buffer_set_description, void)
}

DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::transfer::transfer_source_buffer_set,
                          j::rendering::vulkan::transfer::transfer_source_buffer_set_description);
