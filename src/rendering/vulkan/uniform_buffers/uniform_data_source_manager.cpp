#include "rendering/vulkan/uniform_buffers/uniform_data_source_manager.hpp"

#include "rendering/vulkan/uniform_buffers/uniform_data_source_handler.hpp"
#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"

#include "services/interface_definition.hpp"
#include "services/service_definition.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    void uniform_data_source_manager::initialize(vector<mem::shared_ptr<uniform_data_source_handler>> && handlers) noexcept
    {
      m_handlers[0] = mem::wrap_shared<uniform_data_source_handler, &mem::null_deleter>(
        &renderer_state_uniform_handler::instance);
      data_sources::source_manager<uniform_data_source_key, uniform_buffer_layout, uniform_view>::initialize(
        1, static_cast<vector<mem::shared_ptr<uniform_data_source_handler>> &&>(handlers));
    }

    uniform_data_source_manager uniform_data_source_manager::instance;

    namespace {
      namespace s = services;
      J_A(ND, NODESTROY) s::service_definition<uniform_data_source_manager> def(
        "rendering.vulkan.uniform_data_source_manager",
        "Uniform buffer data source manager",
        s::global_singleton,
        s::create = &uniform_data_source_manager::instance,
        s::initialize = s::initializer_call(&uniform_data_source_manager::initialize),
        s::initialize_by_default = s::initialization_stage::after_dynamic_configuration);

      J_A(ND, NODESTROY) const s::interface_definition<uniform_data_source_handler> def2("rendering.vulkan.uniform_data_source_handler", "Uniform buffer data source handler");
    }
  }
}
