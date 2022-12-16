#include "fonts/selection/font_selector.hpp"
#include "services/interface_definition.hpp"

namespace j::fonts::selection {
  font_selector::~font_selector() {
  }

  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<font_selector> def("fonts.selector", "Font selector");
  }
}
