#pragma once

#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/debug/object_types.hpp"
#include "rendering/vulkan/physical_devices/physical_device_resource.hpp"
#include "rendering/vulkan/samplers/sampler_resource.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_set_resource.hpp"

namespace j::mem {
  template<typename> class shared_ptr;
}

namespace j::events {
  struct event_loop;
}

namespace j::rendering::vulkan {
  class instance;
  namespace shaders  { class shader_manager; }
  namespace surfaces { struct vulkan_window; }
  inline namespace debug { class debug_utils; }
}

namespace j::windowing::x11 {
  class context;
}

namespace j::rendering::vulkan::inline context {
  class device_context_base {
  public:
    virtual mem::shared_ptr<surfaces::vulkan_window> create_window(u16_t width, u16_t height) const = 0;

    /// Get the (logical) device of the context.
    device & device() const noexcept;

    /// Get a reference to the (logical) device of the context.
    virtual const device_ref & device_ref() const noexcept = 0;

    samplers::sampler & default_sampler() const noexcept;

    virtual const samplers::sampler_ref & default_sampler_ref() const noexcept = 0;

    template<typename Handle>
    J_ALWAYS_INLINE void set_object_name(Handle handle, const char * name) const
    { set_object_name(vk_object_type_v<Handle>, (u64_t)handle, name); }

    void set_object_name(VkObjectType type, uptr_t handle, const char * name) const;

    virtual vulkan::instance & instance() const noexcept = 0;

    virtual windowing::x11::context & windowing_context() const noexcept = 0;

    virtual events::event_loop & event_loop() const noexcept = 0;

    virtual shaders::shader_manager & shader_manager() const noexcept = 0;

    virtual debug_utils * debug_utils() const noexcept = 0;

    virtual physical_devices::physical_device & physical_device() const noexcept = 0;

    /// Get the buffer set used for transfers.
    virtual transfer::transfer_source_buffer_set & transfer_source_buffer_set() const noexcept = 0;

    /// Access a resource, ensuring that it is initialized.
    template<typename Ref, typename Resource = typename Ref::resource_t>
    Resource & get(const Ref & ref) const {
      const auto w = ref.wrapper;
      J_ASSERT_NOT_NULL(w);
      if (J_UNLIKELY(w->status != resources::wrapper_status::initialized)) {
        Ref::resource_definition_t::initialize_resource(*this, const_cast<Ref*>(&ref));
      }
      // Note that initialization may have changed the wrapper due to caching.
      return *reinterpret_cast<Resource*>(ref.wrapper->data);
    }

    virtual ~device_context_base();

    device_context_base(const device_context_base &) = delete;
    device_context_base & operator=(const device_context_base &) = delete;
  protected:
    device_context_base() noexcept = default;
  };
}
