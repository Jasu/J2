#include "services/configuration/configuration_pass.hpp"
#include "services/interface_definition.hpp"

namespace j::services::configuration {
  configuration_pass::~configuration_pass() {
  }

  namespace {
    J_A(ND, NODESTROY) const interface_definition<configuration_pass> configuration_pass_definition("configuration.pass", "Configuration pass");
  }
}
