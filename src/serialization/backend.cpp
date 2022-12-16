#include "serialization/backend.hpp"
#include "services/interface_definition.hpp"

namespace j::serialization {
  backend::~backend() {
  }

  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<backend> backend_definition("serialization.backend", "Serialization backend");
  }
}
