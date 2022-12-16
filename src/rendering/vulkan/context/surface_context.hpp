#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "rendering/vulkan/rendering/render_pass_resource.hpp"
#include "device_context_base.hpp"

namespace j::rendering::vulkan {
  struct device;
  class instance;
}
namespace j::rendering::vulkan::surfaces {
  struct surface;
}
namespace j::rendering::vulkan::transfer { class transfer_source_buffer_set; }

namespace j::rendering::vulkan::inline context {
  class surface_context : public device_context_base {
  public:
    const rendering::render_pass  & render_pass() const;

    rendering::render_pass_ref render_pass_ref() const noexcept;

    vulkan::instance & instance() const noexcept override;

    const vulkan::device_ref & device_ref() const noexcept override;

    const samplers::sampler_ref & default_sampler_ref() const noexcept override;

    physical_devices::physical_device & physical_device() const noexcept override;

    transfer::transfer_source_buffer_set & transfer_source_buffer_set() const noexcept override;

    mem::shared_ptr<surfaces::vulkan_window> create_window(u16_t width, u16_t height) const override;

    windowing::x11::context & windowing_context() const noexcept override;

    events::event_loop & event_loop() const noexcept override;

    shaders::shader_manager & shader_manager() const noexcept override;

    class debug_utils * debug_utils() const noexcept override;

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

    virtual ~surface_context();

    surface_context(const mem::shared_ptr<const device_context_base> & device_context_base,
                    surfaces::surface & surface) noexcept;

    surface_context(const surface_context &) = delete;
    surface_context & operator=(const surface_context &) = delete;
    surfaces::surface * surface;
  private:
    mem::shared_ptr<const device_context_base> m_device_context;
  };
}
