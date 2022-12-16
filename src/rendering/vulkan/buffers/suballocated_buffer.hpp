#pragma once

#include "rendering/vulkan/buffers/base_host_visible_buffer.hpp"
#include "rendering/vulkan/memory/free_list_allocator.hpp"
#include "rendering/vulkan/buffers/suballocation.hpp"

namespace j::rendering::vulkan::buffers {
  template<typename Ref>
  class suballocated_buffer : public base_host_visible_buffer {
  public:
    suballocated_buffer(const device_context_base & context,
                        VkBufferUsageFlags usage,
                        u32_t size)
      : base_host_visible_buffer(context, usage, size),
        m_allocator(size)
    {
    }

    /// Allocate memory for a transfer operation from the source buffer.
    suballocation<Ref> allocate(u32_t size) {
      J_ASSERT_NOT_NULL(size);
      const auto res = m_allocator.allocate(size);
      if (J_LIKELY(res)) {
        auto * generic = g_suballocation_pool.allocate();
        J_ASSERT_NOT_NULL(generic);
        generic->offset = res.offset();
        generic->size = size;
        generic->set_buffer<Ref>(static_cast<typename Ref::resource_t*>(this));
        return suballocation<Ref>{generic};
      } else {
        return suballocation<Ref>{};
      }
    }

  private:
    memory::free_list_allocator m_allocator;
    friend class suballocation<Ref>;
  };

}
