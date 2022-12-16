#pragma once

#include "services/interface_definition.hpp"
#include "services/service_definition.hpp"
#include "util/singleton.hpp"
#include "services/service_instance.hpp"

extern template struct j::util::singleton<j::services::container>;
namespace j::services {
  extern constinit j::util::singleton<container> service_container;
}
