#pragma once

#include "services/configuration/configuration_pass.hpp"

#include "properties/wrappers/stored_wrapper.hpp"
#include "properties/access/map_access.hpp"
#include "files/paths/path.hpp"
#include "containers/deque_fwd.hpp"
#include "containers/vector.hpp"

namespace j::application {
  struct information;
}

namespace j::serialization {
  class backend;
}

namespace j::services::configuration {
  /// Pass that reads files based on application configuration.
  class file_configuration_pass final : public configuration_pass {
    struct configuration_file {
      properties::access::map_access interfaces;
      properties::access::map_access services;
      properties::wrappers::stored_wrapper config;
      files::path path;
    };

    using backends_t = vector<const serialization::backend *>;
    backends_t m_backends;
    deque<configuration_file> m_configuration_files;
  public:
    file_configuration_pass(const backends_t & backends, const application::information * info);

    void configure_interface(
      const strings::string & interface_name,
      configuration_union & configuration
    ) const override;

    void before_configure_services() override;

    void preconfigure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      const services::configuration::configuration_union & configuration
    ) override;

    void configure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      configuration_union & configuration
    ) const override;
  };
}
