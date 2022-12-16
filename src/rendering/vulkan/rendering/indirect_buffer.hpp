#pragma once

#include "rendering/vulkan/buffers/base_host_visible_buffer.hpp"
#include "rendering/vulkan/buffers/buffer_access.hpp"

extern template class j::rendering::vulkan::buffers::detail::buffer_access<
  VkDrawIndirectCommand,
  sizeof(VkDrawIndirectCommand),
  j::rendering::vulkan::buffers::base_host_visible_buffer
>;

namespace j::rendering::vulkan::rendering {
  class indirect_buffer_description;

  class indirect_buffer
    : public buffers::detail::buffer_access<
        VkDrawIndirectCommand,
        sizeof(VkDrawIndirectCommand),
        buffers::base_host_visible_buffer
      >
  {
  public:
    indirect_buffer(const device_context_base & context,
                    const indirect_buffer_description & desc);
  };

  class indirect_count_buffer : public buffers::base_host_visible_buffer
  {
    constexpr static VkDeviceSize offset_v = sizeof(u32_t);
  public:
    indirect_count_buffer(const device_context_base & context,
                          const indirect_buffer_description & desc);

    void set_count(u32_t count) {
      auto beg = buffer_begin();
      J_ASSUME_NOT_NULL(beg);
      *reinterpret_cast<u32_t*>(beg) = count;
    }

    sz_t size() const noexcept {
      return (size_bytes - offset_v) / sizeof(VkDrawIndirectCommand);
    }

    template<u32_t I>
    void assign(const VkDrawIndirectCommand (& data)[I]) {
      ::j::memcpy(buffer_begin() + offset_v, data, I * sizeof(VkDrawIndirectCommand));
      set_count(I);
    }

    template<sz_t I>
    void assign_at(VkDeviceSize index, const VkDrawIndirectCommand (& data)[I]) {
      ::j::memcpy(buffer_begin() + offset_v + index * sizeof(VkDrawIndirectCommand),
                  data, I * sizeof(VkDrawIndirectCommand));
    }

    VkDrawIndirectCommand & operator[](VkDeviceSize i) {
      return reinterpret_cast<VkDrawIndirectCommand*>(buffer_begin() + offset_v)[i];
    }

    const VkDrawIndirectCommand & operator[](VkDeviceSize i) const {
      return reinterpret_cast<const VkDrawIndirectCommand*>(buffer_begin() + offset_v)[i];
    }
  };
}
