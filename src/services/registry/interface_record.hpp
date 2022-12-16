#pragma once

#include "services/configuration/interface_configuration.hpp"

namespace j::services {
  struct class_metadata;
}
namespace j::services::registry {
  struct interface_record {
    const class_metadata * metadata;
    configuration::interface_configuration default_configuration;
  };
}
