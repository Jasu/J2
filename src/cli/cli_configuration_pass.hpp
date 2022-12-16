#pragma once

#include "services/configuration/configuration_pass.hpp"

#include "containers/unsorted_string_map_fwd.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::cli {
  class application;

  struct option_record;
  class cli_configuration_pass final : public services::configuration::configuration_pass {
  public:
    explicit cli_configuration_pass(mem::shared_ptr<application> app);

    cli_configuration_pass(cli_configuration_pass &&) = delete;
    cli_configuration_pass(const cli_configuration_pass &) = delete;
    cli_configuration_pass & operator=(cli_configuration_pass &&) = delete;
    cli_configuration_pass & operator=(const cli_configuration_pass &) = delete;

    /// NOP for CLI configuration.
    void configure_interface(
      const strings::string &,
      services::configuration::configuration_union &
    ) const noexcept override;

    void preconfigure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      const services::configuration::configuration_union & configuration
    ) override;

    void before_configure_services() override;

    void configure_service(
      const strings::string & service_name,
      const strings::string & setting_name,
      services::configuration::configuration_union & configuration
    ) const override;

  private:
    mem::shared_ptr<application> m_application;
    unsorted_string_map<unsorted_string_map<vector<option_record>>> m_options;
  };
}
