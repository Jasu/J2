#pragma once

namespace j::strings {
  class string;
}

namespace j::services::configuration {
  class configuration_union;

  /// Interface for configuration passes.
  ///
  /// Configuration passes are run one by one, building the configuration.
  class configuration_pass {
  public:
    virtual ~configuration_pass();

    /// Adds an configuration overlay for the interface specified, if configured.
    virtual void configure_interface(
      const strings::string & interface_name,
      configuration_union & configuration) const = 0;

    /// Prepare for configurating service.
    ///
    /// This function is called for each service to be configured before configure_service is called
    /// for any of them. For example, the CLI configuration pass uses this to configure the CLI
    /// library with all available configuration options.
    virtual void preconfigure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      const configuration_union & configuration) = 0;

    /// Called after the last preconfigure_service() and before calling configure_service.
    virtual void before_configure_services() = 0;

    /// Configuration overlay for the service specified.
    virtual void configure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      configuration_union & configuration) const = 0;
  };
}
