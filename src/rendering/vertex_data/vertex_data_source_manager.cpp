#include "rendering/vertex_data/vertex_data_source_manager.hpp"

#include "mem/shared_ptr.hpp"
#include "services/service_definition.hpp"
#include "services/interface_definition.hpp"
#include "rendering/vertex_data/static_vertex_data_source_handler.hpp"

namespace j::rendering::vertex_data {
  void vertex_data_source_manager::initialize(vector<mem::shared_ptr<vertex_data_source_handler>> && handlers) noexcept
  {
    m_handlers[0] = mem::wrap_shared<vertex_data_source_handler, &mem::null_deleter>(&static_vertex_data_source_handler::instance);
    data_sources::source_manager<vertex_data_source_key, vertex_buffer_info, vertex_buffer_view>::initialize(
      1, static_cast<vector<mem::shared_ptr<vertex_data_source_handler>> &&>(handlers));
  }

  vertex_data_source_manager vertex_data_source_manager::instance;

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<vertex_data_source_manager> def(
      "rendering.vertex_data_source_manager",
      "Vertex data source manager",
      s::create = &vertex_data_source_manager::instance,
      s::initialize = s::initializer_call(&vertex_data_source_manager::initialize),
      s::initialize_by_default = s::initialization_stage::after_dynamic_configuration
    );
    J_A(ND, NODESTROY) const s::interface_definition<vertex_data_source_handler> def2("rendering.vertex_data_source_handler", "Vertex data source handler");
  }
}
