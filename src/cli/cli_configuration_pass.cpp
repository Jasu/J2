#include "cli/cli_configuration_pass.hpp"

#include "cli/application.hpp"
#include "cli/metadata.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "services/configuration/configuration_union.hpp"
#include "properties/visiting/visit_definition_recursive.hpp"
#include "properties/access_by_path.hpp"
#include "services/service_definition.hpp"
#include "exceptions/assert_lite.hpp"
#include "containers/unsorted_string_map.hpp"
#include "properties/path.hpp"

namespace j::cli {
  struct option_record {
    J_BOILERPLATE(option_record, CTOR_CE)

    properties::path path;
    u32_t index = 0U;

    template<typename Path>
    option_record(Path && path, u32_t index)
      : path(static_cast<Path &&>(path)),
        index(index)
    { }
  };

  namespace {
    properties::path make_path(const properties::visiting::visit_definition_path * path) {
      switch (path->type()) {
      case properties::visiting::DEFINITION_COMPONENT_TYPE::ROOT:
        return {};
      case properties::visiting::DEFINITION_COMPONENT_TYPE::OBJECT_PROPERTY: {
        auto p = make_path(path->parent());
        p.emplace_back(properties::property_name_tag, path->property()->name());
        return p;
      }
      default:
        // TODO: Make this work with arrays etc.
        J_THROW("Unimplemented");
      }
    }
  }

  cli_configuration_pass::cli_configuration_pass(mem::shared_ptr<application> app)
    : m_application(static_cast<mem::shared_ptr<application> &&>(app))
  {
    J_ASSERT_NOT_NULL(m_application);
  }

  void cli_configuration_pass::configure_interface(
    const strings::string &,
    services::configuration::configuration_union &
  ) const noexcept {
  }

  void cli_configuration_pass::preconfigure_service(
    const strings::string & service_name,
    const strings::string & setting_name,
    const services::configuration::configuration_union & configuration
  ) {
    properties::visiting::visit_definition_recursive(
      configuration.definition(),
      [this, &service_name, &setting_name](const properties::visiting::visit_definition_path & path) {
        if (path.is_root() || !path.is_object_property()) {
          return;
        }
        if (!path.property()->metadata().contains(cli_option)) {
          return;
        }
        const char *option_name = path.property()->metadata().get(cli_option),
                   *short_desc = nullptr, *desc = nullptr;

        if (path.property()->metadata().contains(short_description)) {
          short_desc = path.property()->metadata().get(short_description);
        }
        if (path.property()->metadata().contains(description)) {
          desc = path.property()->metadata().get(description);
        }
        m_application->add_option(
          option_name,
          {
            .index = m_application->m_option_counter,
            .is_option = true,
            .has_value = true,
            .short_description = short_desc,
            .long_description = desc
          });
        m_options[service_name][setting_name].emplace_back(
          make_path(&path),
          m_application->m_option_counter++);
      });
  }

  void cli_configuration_pass::before_configure_services() {
    m_application->parse();
  }

  void cli_configuration_pass::configure_service(
    const strings::string & service_name,
    const strings::string & setting_name,
    services::configuration::configuration_union & configuration
  ) const {
    auto it = m_options.find(service_name);
    if (it == m_options.end()) {
      return;
    }
    auto it2 = it->second->find(setting_name);
    if (it2 == it->second->end()) {
      return;
    }
    auto & options = *it2->second;
    bool has = false;
    for (auto & option : options) {
      if (m_application->has_option(option.index)) {
        has = true;
        break;
      }
    }

    if (!has) {
      return;
    }

    services::configuration::configuration_overlay::paths_t paths;
    mem::shared_ptr<void> value{configuration.create_default_configuration()};
    properties::wrappers::stored_wrapper wrapper(
      properties::wrappers::wrapper(configuration.definition(), value.get()),
      static_cast<mem::shared_ptr<void> &&>(value)
    );
    for (auto & option : options) {
      auto s = m_application->get_option(option.index);
      if (!s) {
        continue;
      }
      paths.insert(option.path);
      auto prop = properties::access_by_path(
        wrapper.wrapper(),
        option.path,
        properties::access_by_path_flag::create);
      for (auto content : prop.definition().possible_contents()) {
        switch (content->type) {
        case properties::PROPERTY_TYPE::BOOL:
          prop.set_value((bool)::j::strcmp(s.front(), "true") && (bool)::j::strcmp(s.front(), "0"));
          break;
        default:
          J_THROW("Unimplemented - do this properly.");
        }
      }
    }

    configuration.add_overlay(
      static_cast<services::configuration::configuration_overlay::paths_t &&>(paths),
      static_cast<properties::wrappers::stored_wrapper &&>(wrapper),
      2);
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<cli_configuration_pass> cli_configuration_pass_definition(
      "services.configuration.pass.cli",
      "CLI configuration pass",
      s::no_interface,
      s::implements = s::interface<s::configuration::configuration_pass>(0),
      s::create = s::constructor<mem::shared_ptr<application>>()
    );
  }
}
