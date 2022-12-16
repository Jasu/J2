#include "fonts/rasterizing/font_rasterizer.hpp"
#include "services/interface_definition.hpp"

namespace j::fonts::rasterizing {
  font_rasterizer::~font_rasterizer() {
  }

  namespace {
    J_A(ND, NODESTROY) services::interface_definition<font_rasterizer> def("fonts.rasterizer", "Font rasterizer");
  }
}
