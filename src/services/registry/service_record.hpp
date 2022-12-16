#pragma once

#include "services/container/initialization_stage.hpp"

namespace j::mem {
  template<typename> class shared_ptr;
}
namespace j::inline containers {
  template<typename> class trivial_array;
}

namespace j::services {
  class container;
  struct injection_context;
  struct class_metadata;
}

namespace j::services::detail {
  struct setting;
}
namespace j::services::registry {
  struct implements_record;

  /// Factory function type for services.
  using service_getter = mem::shared_ptr<void> (*)(container*, const injection_context*);

  struct service_record {
    const class_metadata * metadata;
    service_getter get;
    const trivial_array<detail::setting> * settings;
    const trivial_array<implements_record> * implements;
    initialization_stage initialization_stage;
    const class_metadata * default_instance;
  };
}
