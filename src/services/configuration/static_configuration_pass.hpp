#pragma once

#include "services/configuration/configuration_pass.hpp"
#include "properties/wrappers/stored_wrapper.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "containers/vector.hpp"
#include "strings/string_view.hpp"

namespace j::properties::wrappers {
  class stored_wrapper;
}

namespace j::services::configuration {
  /// Configuration pass used for passing static configuration to the container.
  ///
  /// This is not an injectable service, but is used by the container before actually
  /// initializing the container.
  class static_configuration_pass final : public configuration_pass {
  public:
    using setting_vector_t = vector<properties::wrappers::stored_wrapper>;
    using settings_t = unsorted_string_map<setting_vector_t>;
    using service_settings_t = unsorted_string_map<settings_t>;
  private:
    settings_t m_interface_configuration;
    service_settings_t m_service_configuration;
  public:
    void add_service_configuration_wrapper(
      strings::const_string_view service_name,
      strings::const_string_view setting_name,
      properties::wrappers::stored_wrapper && wrapper);

    void add_interface_configuration_wrapper(
      strings::const_string_view interface_name,
      properties::wrappers::stored_wrapper && wrapper);

    void configure_interface(const strings::string & interface_name, configuration_union & configuration) const override;

    void before_configure_services() override;

    void preconfigure_service(const strings::string & service_name, const strings::string & setting_name, const configuration_union & configuration) override;

    void configure_service(const strings::string & service_name, const strings::string & setting_name, configuration_union & configuration) const override;

    static_configuration_pass() noexcept;

    ~static_configuration_pass();
  };
}
