#include "rendering/vulkan/resources/resource_ref.hpp"

namespace j::rendering::vulkan::resources::detail {
  [[nodiscard]] J_RETURNS_NONNULL resource_wrapper * resource_ref_weak_base::containing_record(const void * J_NOT_NULL resource) noexcept {
    J_ASSUME_NOT_NULL(resource);
    sz_t data_offset = reinterpret_cast<sz_t>(static_cast<resource_wrapper*>(nullptr)->data);
    u8_t * resource_ptr = reinterpret_cast<u8_t*>(const_cast<void*>(resource));
    return reinterpret_cast<resource_wrapper*>(resource_ptr - data_offset);
  }

  resource_ref_base::resource_ref_base(const resource_ref_base & rhs) noexcept
    : resource_ref_weak_base(rhs.wrapper)
  {
    if (wrapper) {
      ++wrapper->refcount;
    }
  }
}
