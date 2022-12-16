#include "services/service_reference.hpp"
#include "services/stored_service_reference.hpp"
#include "strings/string.hpp"

namespace j::services {
  type_id::type_id service_reference::service_type_id() const noexcept {
    return m_service_type_id_getter();
  }

  bool service_reference::empty() const noexcept {
    return m_service_type_id_getter().empty() && m_service_instance_name.empty();
  }

  bool service_reference::operator==(const service_reference & rhs) const noexcept {
    return
      m_service_type_id_getter() == rhs.m_service_type_id_getter()
      && m_service_instance_name == rhs.m_service_instance_name;
  }

  bool service_reference::operator!=(const service_reference & rhs) const noexcept {
    return !operator==(rhs);
  }

  bool service_reference::operator<(const service_reference & rhs) const noexcept {
    if (m_service_type_id_getter() < rhs.m_service_type_id_getter()) {
      return true;
    }
    return m_service_type_id_getter() == rhs.m_service_type_id_getter()
      && m_service_instance_name < rhs.m_service_instance_name;
  }

  bool service_reference::operator>(const service_reference & rhs) const noexcept {
    return !operator<(rhs) && !operator==(rhs);
  }

  bool service_reference::operator<=(const service_reference & rhs) const noexcept {
    return !operator<(rhs) || !operator==(rhs);
  }

  bool service_reference::operator>=(const service_reference & rhs) const noexcept {
    return !operator<(rhs);
  }
}
