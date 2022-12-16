#pragma once

#include "rendering/vulkan/buffers/base_host_visible_buffer.hpp"
#include "bits/bitset.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    struct uniform_buffer_record;
  }
}
J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::uniform_buffer_record);

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class uniform_buffer_allocation;
    struct uniform_buffer_record;
    struct uniform_buffer_layout;
    struct uniform_buffer_description;

    class uniform_buffer final : public buffers::base_host_visible_buffer {
    public:
      uniform_buffer(const device_context_base & context,
                     uniform_buffer_description && description);

      ~uniform_buffer();

      uniform_buffer_allocation allocate() noexcept;
      uniform_buffer_allocation allocate(u32_t layout_index) noexcept;

      const uniform_buffer_layout & layout(u32_t layout_index) noexcept;

      u32_t allocation_offset(u16_t allocation_index);
      u32_t allocation_size(u16_t allocation_index);

      uniform_buffer(uniform_buffer &&) = delete;
      uniform_buffer & operator=(uniform_buffer &&) = delete;
    private:
      bits::bitset m_allocations;
      J_PACKED trivial_array<uniform_buffer_record> m_records;
      u32_t m_alignment;

      friend class uniform_buffer_allocation;
    };
  }
}
