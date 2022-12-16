#include "rendering/vulkan/surfaces/window_factory.hpp"

#include "services/interface_definition.hpp"

namespace j::rendering::vulkan::surfaces {
  window_factory::~window_factory() {
  }

  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<window_factory> def("vulkan.window_factory", "Window factory");
  }
}
