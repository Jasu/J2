#include "services/container/service_instance_state.hpp"

#include "containers/trivial_array.hpp"
#include "services/container/service_type.hpp"
#include "services/exceptions.hpp"
#include "services/registry/service_record.hpp"
#include "services/metadata.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "services/detail/setting.hpp"
#include "services/configuration/configuration_pass.hpp"
#include "containers/unsorted_string_map.hpp"
#include "mem/weak_ptr.hpp"

J_DEFINE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::services::detail::container::service_setting);

namespace j::services::detail::container {
  service_instance_state::service_instance_state() {
  }

  service_instance_state::service_instance_state(
    const registry::service_record * J_NOT_NULL record,
    const service_type * J_NOT_NULL type,
    const class_metadata * J_NOT_NULL metadata
  ) : m_service_type(type),
      m_metadata(metadata)
  {
    for (auto & setting : *record->settings) {
      m_settings.emplace(setting.name, service_setting{
        &setting,
        configuration::configuration_union(
          setting.type_id(),
          configuration::config_factory_t(setting.create_configuration))
      });
    }
  }

  service_instance_state::service_instance_state(service_instance_state && rhs) noexcept
    : m_instance(static_cast<mem::weak_ptr<void> &&>(rhs.m_instance)),
      m_service_type(rhs.m_service_type),
      m_metadata(rhs.m_metadata),
      m_settings(static_cast<containers::unsorted_string_map<service_setting> &&>(rhs.m_settings))
  {
  }

  service_instance_state::service_instance_state(const service_instance_state & rhs)
    : m_instance(rhs.m_instance),
      m_service_type(rhs.m_service_type),
      m_metadata(rhs.m_metadata),
      m_settings(rhs.m_settings)
  {
  }

  service_instance_state & service_instance_state::operator=(service_instance_state && rhs) noexcept {
    if (this != &rhs) {
      m_instance = static_cast<mem::weak_ptr<void> &&>(rhs.m_instance);
      m_service_type = rhs.m_service_type;
      m_metadata = rhs.m_metadata;
      m_settings = static_cast<containers::unsorted_string_map<service_setting> &&>(rhs.m_settings);
    }
    return *this;
  }

  service_instance_state & service_instance_state::operator=(const service_instance_state & rhs) {
    if (this != &rhs) {
      m_instance = rhs.m_instance;
      m_service_type = rhs.m_service_type;
      m_metadata = rhs.m_metadata;
      m_settings = rhs.m_settings;
    }
    return *this;
  }

  service_instance_state::~service_instance_state() {
  }

  type_id::type_id service_instance_state::type_id() const noexcept {
    return m_service_type->type_id();
  }

  mem::shared_ptr<void> service_instance_state::get_if_instantiated() const noexcept {
    return m_instance.lock();
  }

  mem::shared_ptr<void> service_instance_state::get(services::container * c, injection_context * ic) {
    mem::shared_ptr<void> result{m_instance.lock()};
    if (result) {
      return result;
    }
    return create_instance(c, ic);
  }

  mem::shared_ptr<void> service_instance_state::create_instance(services::container * J_NOT_NULL c, injection_context * J_NOT_NULL ic) {
    ic->service_instance_state = this;
    ic->service_metadata = m_metadata;
    mem::shared_ptr<void> result{m_service_type->m_record->get(c, ic)};
    J_REQUIRE_EX(result, exception()
                 << message("Service factory returned null.")
                 << err_service_name(m_service_type->get_name())
                 << err_service_instance_name(m_metadata->name));
    m_instance = result;
    return result;
  }

  void service_instance_state::preconfigure(configuration::configuration_pass & pass) const {
    if (m_settings.empty()) {
      return;
    }
    const strings::string name{m_metadata->name};
    for (auto & p : m_settings) {
      pass.preconfigure_service(name, *p.first, p.second->value);
    }
  }

  void service_instance_state::configure(configuration::configuration_pass & pass) {
    if (m_settings.empty()) {
      return;
    }
    const strings::string name{m_metadata->name};
    for (auto & p : m_settings) {
      pass.configure_service(name, *p.first, p.second->value);
    }
  }

  bool service_instance_state::has_setting(const type_id::type_id & setting_type, const char * name) const noexcept {
    J_ASSERT_NOT_NULL(name);
    auto setting = m_settings.maybe_at(name);
    if (!setting) {
      return false;
    }
    return setting->setting->type_id() == setting_type;
  }

  bool service_instance_state::has_setting(const std::type_info & setting_type, const char * name) const noexcept {
    return has_setting(type_id::type_id(setting_type), name);
  }

  mem::shared_ptr<void> service_instance_state::get_setting([[maybe_unused]] const type_id::type_id & setting_type,
                                                            const char * J_NOT_NULL name) {
    auto it = m_settings.find(name);
    J_ASSERT_EX(it != m_settings.end(), exception()
                << message("Setting not found by name.")
                << err_setting_name(name));
    J_ASSERT_EX(it->second->setting->type_id() == setting_type, exception()
                << message("Setting type does not match.")
                << err_setting_name(name));
    mem::shared_ptr<void> configuration_ptr{it->second->value.create_configuration()};
    J_ASSERT_EX(configuration_ptr, exception()
                << message("Service configuration is a null pointer.")
                << err_setting_name(name));
    return configuration_ptr;
  }

  mem::shared_ptr<void> service_instance_state::get_setting(const std::type_info & setting_type,
                                                            const char * J_NOT_NULL name) {
    return get_setting(type_id::type_id(setting_type), name);
  }

  const char * service_instance_state::name() const noexcept {
    return m_metadata->name;
  }
}
