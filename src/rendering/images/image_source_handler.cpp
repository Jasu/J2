#include "rendering/images/image_source_handler.hpp"
#include "services/interface_definition.hpp"

namespace j::rendering::images {
  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<image_source_handler> def(
      "rendering.image_source_handler",
      "Image data source handler");
  }
}
