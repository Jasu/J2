#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "type_id/type_id.hpp"

namespace j::services::detail {
  using setting_factory = mem::shared_ptr<void> (*)(void);

  /// Attribute describing a setting.
  struct setting {
    const char * name;
    type_id::type_id_getter type_id;
    setting_factory create_configuration;
  };
}
