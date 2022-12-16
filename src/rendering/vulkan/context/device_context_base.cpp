#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/samplers/sampler.hpp"
#include "rendering/vulkan/debug/debug_utils.hpp"

namespace j::rendering::vulkan::inline context {
  device & device_context_base::device() const noexcept {
    return get(device_ref());
  }

  samplers::sampler & device_context_base::default_sampler() const noexcept {
    return get(default_sampler_ref());
  }

  void device_context_base::set_object_name(VkObjectType type, uptr_t handle, const char * name) const {
    if (!name) {
      return;
    }
    if (auto u = debug_utils()) {
      u->set_object_name(*this, type, handle, name);
    }
  }

  device_context_base::~device_context_base() {
  }
}
