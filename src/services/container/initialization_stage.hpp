#pragma once

namespace j::services {
  enum class initialization_stage {
    /// Default initialization stage for a service - it is initialized when needed.
    lazy,
    /// Earliest possible initialization stage, before static configuration has been run.
    before_static_configuration,
    /// After static configuration, but before dynamic configuration.
    after_static_configuration,
    /// After dynamic configuration, when the container is initialized.
    after_dynamic_configuration,
  };
}
