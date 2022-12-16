#include "services/configuration/static_configuration_pass.hpp"
#include "services/configuration/configuration_union.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/vector.hpp"
#include "strings/string_view.hpp"

namespace j::services::configuration {
  void static_configuration_pass::add_service_configuration_wrapper(
    strings::const_string_view service_name,
    strings::const_string_view setting_name,
    properties::wrappers::stored_wrapper && wrapper)
  {
    m_service_configuration[service_name][setting_name].emplace_back(static_cast<properties::wrappers::stored_wrapper &&>(wrapper));
  }

  void static_configuration_pass::add_interface_configuration_wrapper(
    strings::const_string_view interface_name,
    properties::wrappers::stored_wrapper && wrapper)
  {
    m_interface_configuration[interface_name].emplace_back(static_cast<properties::wrappers::stored_wrapper &&>(wrapper));
  }

  void static_configuration_pass::configure_interface(const strings::string & interface_name, configuration_union & configuration) const {
    if (auto it = m_interface_configuration.find(interface_name); it != m_interface_configuration.end()) {
      for (const auto & setting : *it->second) {
        configuration.add_overlay(setting, 1);
      }
    }
  }

  void static_configuration_pass::before_configure_services() { }

  void static_configuration_pass::preconfigure_service(
    const strings::string &, const strings::string &, const configuration_union &)
  { }

  void static_configuration_pass::configure_service(
    const strings::string & service_name,
    const strings::string & setting_name,
    configuration_union & configuration) const
  {
    if (auto it = m_service_configuration.find(service_name); it != m_service_configuration.end()) {
      if (auto it2 = it->second->find(setting_name); it2 != it->second->end()) {
        for (const auto & setting : *it2->second) {
          configuration.add_overlay(setting, 1);
        }
      }
    }
  }

  static_configuration_pass::static_configuration_pass() noexcept {
  }

  static_configuration_pass::~static_configuration_pass() {
  }
}
