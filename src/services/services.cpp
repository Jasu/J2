#include "services/services.hpp"
#include "services/configuration/static_configuration_pass.hpp"

template struct j::util::singleton<j::services::container>;

namespace j::services {
  constinit j::util::singleton<container> service_container;
}
