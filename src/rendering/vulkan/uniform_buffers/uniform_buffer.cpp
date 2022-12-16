#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"

#include "rendering/vulkan/uniform_buffers/uniform_buffer_description.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::uniform_buffer_record);

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    namespace {
      u32_t calculate_size(const trivial_array<uniform_buffer_record> & records,
                           u32_t alignment, u32_t up_to = U32_MAX) noexcept
      {
        u32_t result = 0U;
        for (auto & p : records) {
          if (!up_to--) { break; }
          J_ASSERT_NOT_NULL(p.count, p.layout.size);
          result += p.count * j::max(p.layout.size, alignment);
        }
        return result;
      }

      u32_t count_records(const trivial_array<uniform_buffer_record> & records,
                          u32_t up_to = U32_MAX) noexcept
      {
        u32_t result = 0U;
        for (auto & p : records) {
          if (!up_to--) { break; }
          result += p.count;
        }
        return result;
      }
    }

    uniform_buffer::uniform_buffer(const device_context_base & context,
                                   uniform_buffer_description && description)
      : base_host_visible_buffer(
          context,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          calculate_size(
            description.records,
            context.physical_device().properties().limits.minUniformBufferOffsetAlignment)
        ),
        m_allocations(count_records(description.records)),
        m_records(static_cast<trivial_array<uniform_buffer_record> &&>(description.records)),
        m_alignment(context.physical_device().properties().limits.minUniformBufferOffsetAlignment)
    {
    }

    uniform_buffer::~uniform_buffer() {
    }

    uniform_buffer_allocation uniform_buffer::allocate() noexcept {
      J_ASSERT(m_records.size() == 1,
               "Allocate without parameters works only with a single record.");
      return allocate(0);
    }

    uniform_buffer_allocation uniform_buffer::allocate(u32_t layout_index) noexcept {
      J_ASSERT(layout_index < m_records.size(),
               "Layout index out of range.");
      const u32_t start_num = count_records(m_records, layout_index);
      const u32_t allocation_index = m_allocations.find_and_set_first_clear(start_num);
      if (J_UNLIKELY(allocation_index == bits::bitset::npos)) {
        return uniform_buffer_allocation();
      }
      const u32_t start_offset = calculate_size(m_records, m_alignment, layout_index);
      return uniform_buffer_allocation(
        uniform_buffer_ref::from_this(this),
        start_offset + j::max(m_alignment, m_records[layout_index].layout.size) * (allocation_index - start_num),
        allocation_index,
        layout_index);
    }

    const uniform_buffer_layout & uniform_buffer::layout(u32_t layout_index) noexcept {
      return m_records.at(layout_index).layout;
    }

    u32_t uniform_buffer::allocation_offset(const u16_t allocation_index) {
      u32_t offset = 0;
      u16_t i = allocation_index;
      for (auto & p : m_records) {
        const u32_t sz = j::max(p.layout.size, m_alignment);
        if (p.count < i) {
          offset += p.count * sz;
          i -= p.count;
        } else {
          return offset + allocation_index * sz;
        }
      }
      J_THROW("Allocation index out of range.");
    }

    u32_t uniform_buffer::allocation_size(const u16_t allocation_index) {
      u16_t i = allocation_index;
      for (auto & p : m_records) {
        if (p.count < i) {
          i -= p.count;
        } else {
          return p.layout.size;
        }
      }
      J_THROW("Allocation index out of range.");
    }

    uniform_buffer_allocation::uniform_buffer_allocation(
      uniform_buffer_ref buffer,
      u32_t offset,
      u16_t allocation_index,
      u16_t layout_index
    ) noexcept
      : buffer(static_cast<uniform_buffer_ref &&>(buffer)),
        offset(offset),
        allocation_index(allocation_index),
        layout_index(layout_index)
    {
      J_ASSERT_NOT_NULL(this->buffer);
      J_ASSERT(layout_index < this->buffer.get().m_records.size(),
               "Layout index out of range.");
      J_ASSERT(allocation_index < this->buffer.get().m_allocations.capacity(),
               "Allocation index out of range.");
      J_ASSERT(this->buffer.get().m_allocations.is_set(allocation_index),
               "Allocation was not set.");
    }

    u8_t * uniform_buffer_allocation::target() noexcept {
      return buffer.get().buffer_data() + offset;
    }

    void uniform_buffer_allocation::release() noexcept {
      J_ASSERT(buffer.get().m_allocations.is_set(allocation_index),
               "Allocation  not set.");
      buffer.get().m_allocations.reset(allocation_index);
      buffer.reset();
      allocation_index = offset = layout_index = 0;
    }
    DEFINE_RESOURCE_DEFINITION(uniform_buffer, uniform_buffer_description, void)
  }
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::uniform_buffer,
                          j::rendering::vulkan::uniform_buffer_description);
