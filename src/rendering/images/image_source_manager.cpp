#include "rendering/images/image_source_manager.hpp"

#include "services/service_definition.hpp"
#include "rendering/images/static_image_source_handler.hpp"

namespace j::rendering::images {
  void image_source_manager::initialize(vector<mem::shared_ptr<image_source_handler>> && handlers) noexcept
  {
    m_handlers[0] = mem::wrap_shared<image_source_handler, &mem::null_deleter>(&static_image_source_handler::instance);
    data_sources::source_manager<image_source_key, image_buffer_info, image_buffer_view>::initialize(
      1, static_cast<vector<mem::shared_ptr<image_source_handler>> &&>(handlers));
  }

  image_source_manager image_source_manager::instance;

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<image_source_manager> def(
      "rendering.image_source_manager",
      "Image source manager",
      s::global_singleton,
      s::create = &image_source_manager::instance,
      s::initialize = s::initializer_call(&image_source_manager::initialize),
      s::initialize_by_default = s::initialization_stage::after_dynamic_configuration
    );
  }
}
