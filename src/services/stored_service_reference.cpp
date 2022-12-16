#include "services/stored_service_reference.hpp"
#include "properties/class_registration.hpp"
#include "properties/access/string_access.hpp"
#include "strings/string.hpp"
#include "services/service_reference.hpp"

namespace j::services {
  stored_service_reference::stored_service_reference() noexcept {
  }

  stored_service_reference::stored_service_reference(const service_reference & rhs)
    : m_type_id(rhs.service_type_id()),
      m_string(rhs.service_instance_name())
  {
  }

  stored_service_reference::stored_service_reference(const stored_service_reference & rhs)
    : m_type_id(rhs.m_type_id),
      m_string(rhs.m_string)
  {
  }

  stored_service_reference::stored_service_reference(stored_service_reference && rhs) noexcept
    : m_type_id(rhs.m_type_id),
      m_string(static_cast<strings::string &&>(rhs.m_string))
  {
  }

  stored_service_reference & stored_service_reference::operator=(const stored_service_reference & rhs) {
    if (this != &rhs) {
      m_type_id = rhs.m_type_id;
      m_string = rhs.m_string;
    }
    return *this;
  }

  stored_service_reference & stored_service_reference::operator=(stored_service_reference && rhs) noexcept {
    if (this != &rhs) {
      m_type_id = rhs.m_type_id;
      m_string = static_cast<strings::string &&>(rhs.m_string);
    }
    return *this;
  }

  stored_service_reference::~stored_service_reference() {
  }

  stored_service_reference::stored_service_reference(const type_id::type_id & type_id, const strings::string & str)
    : m_type_id(type_id),
      m_string(str)
  {
  }

  stored_service_reference::stored_service_reference(const type_id::type_id & type_id, strings::string && str)
    : m_type_id(type_id),
      m_string(static_cast<strings::string &&>(str))
  {
  }

  stored_service_reference::stored_service_reference(const strings::string & str)
    : m_string(str) {
  }

  stored_service_reference::stored_service_reference(strings::string && str) noexcept
    : m_string(static_cast<strings::string &&>(str)) {
  }

  stored_service_reference::stored_service_reference(const type_id::type_id & type_id) noexcept
    : m_type_id(type_id)
  {
  }

  bool stored_service_reference::operator==(const stored_service_reference & rhs) const noexcept {
    return m_type_id == rhs.m_type_id && m_string == rhs.m_string;
  }

  bool stored_service_reference::operator<(const stored_service_reference & rhs) const noexcept {
    return m_type_id < rhs.m_type_id || (m_type_id == rhs.m_type_id && m_string < rhs.m_string);
  }
}

namespace j::services::configuration {
  using namespace properties;

  namespace {
    struct service_reference_access {
      static void * create_from_string(const object_access_definition &, string_access str) {
        // TODO: Handle this properly, i.e. figure out a syntax for specifying
        //       a type or a name.
        return new stored_service_reference(str.as_j_string());
      }

      static void assign_from_string(const object_access_definition &, stored_service_reference & lhs, string_access rhs) {
        lhs = stored_service_reference(rhs.as_j_string());
      }
    };

    object_access_registration<stored_service_reference, service_reference_access> reg{"stored_service_reference"};
  }
}
