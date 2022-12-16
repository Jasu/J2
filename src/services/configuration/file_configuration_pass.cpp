#include "services/configuration/file_configuration_pass.hpp"

#include "services/configuration/configuration_overlay.hpp"
#include "services/configuration/configuration_union.hpp"
#include "application/information.hpp"
#include "serialization/backend.hpp"
#include "containers/deque.hpp"
#include "containers/vector.hpp"
#include "files/fs.hpp"

namespace j::services::configuration {
  file_configuration_pass::file_configuration_pass(const backends_t & backends, const application::information * info)
    : m_backends(backends)
  {
    for (auto & path : info->configuration_files) {
      if (!files::fs::exists(path)) {
        continue;
      }
      for (auto backend : backends) {
        if (backend->supports(path)) {
          auto configuration{backend->read(path)};
          auto & wrapper = configuration.wrapper();
          if (wrapper.is_nil()) {
            break;
          }
          auto map = wrapper.as_map();
          configuration_file file {
            .config = static_cast<properties::wrappers::stored_wrapper &&>(configuration),
            .path = path,
          };
          if (auto it = map.find("services"); it != map.end() && !it->value().is_nil()) {
            file.services = it->value().as_map();
          }
          if (auto it = map.find("interfaces"); it != map.end() && !it->value().is_nil()) {
            file.interfaces = it->value().as_map();
          }
          m_configuration_files.emplace_back(static_cast<configuration_file &&>(file));
          break;
        }
      }
    }
  }

  void file_configuration_pass::configure_interface(const strings::string & interface_name, configuration_union & configuration) const {
    for (auto & file : m_configuration_files) {
      auto interfaces = file.interfaces;
      if (auto it = interfaces.find(interface_name); it != interfaces.end()) {
        configuration.add_overlay(properties::wrappers::stored_wrapper{it->value(), file.config.data() }, 2);
      }
    }
  }

  void file_configuration_pass::before_configure_services()
  { }

  void file_configuration_pass::preconfigure_service(
    const strings::string &, const strings::string &, const configuration_union &)
  { }

  void file_configuration_pass::configure_service(const strings::string & service_name, const strings::string & setting_name, configuration_union & configuration) const {
    for (auto & file : m_configuration_files) {
      auto services = file.services;
      if (auto service = services.find(service_name); service != services.end()) {
        if (!service->value().is_map()) {
          continue;
        }
        auto service_map = service->value().as_map();
        if (auto setting = service_map.find(setting_name); setting != service_map.end()) {
          configuration.add_overlay(properties::wrappers::stored_wrapper{setting->value(), file.config.data() }, 2);
        }
      }
    }
  }
}

#include "services/services.hpp"

namespace j::services::configuration {
  namespace {
    J_A(ND, NODESTROY) service_definition<file_configuration_pass> file_configuration_pass_definition(
      "services.configuration.pass.file",
      "File configuration pass",
      no_interface,
      implements = interface<configuration_pass>(1),
      create = constructor<
        const vector<const serialization::backend *> &,
        const application::information *
      >()
    );
  }
}
