#pragma once

#include "containers/trivial_array_fwd.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_resource.hpp"
#include "rendering/vulkan/transfer/transfer_operation_source.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::transfer::transfer_source_buffer_ref);

namespace j::rendering::vulkan::transfer {
  class transfer_source_buffer_set_description;
  /// Set of transfer source buffers.
  class transfer_source_buffer_set {
  public:
    /// Construct a buffer set with [count] buffers of [size] bytes each.
    transfer_source_buffer_set(const device_context_base & context,
                               const transfer_source_buffer_set_description & desc);

    ~transfer_source_buffer_set();

    /// Allocate memory for a transfer operation from one of the source buffers.
    transfer_operation_source allocate(const device_context_base & context, u32_t size);

    trivial_array<transfer_source_buffer_ref> & buffers() noexcept {
      return m_buffers;
    }

    const trivial_array<transfer_source_buffer_ref> & buffers() const noexcept {
      return m_buffers;
    }

  private:
    trivial_array<transfer_source_buffer_ref> m_buffers;
  };
}
