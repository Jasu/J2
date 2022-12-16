#pragma once

#include "strings/string.hpp"
#include "type_id/type_id.hpp"

namespace j::services {
  class service_reference;

  class stored_service_reference {
  public:
    stored_service_reference() noexcept;

    stored_service_reference(const stored_service_reference & ref);
    stored_service_reference(stored_service_reference && ref) noexcept;
    stored_service_reference & operator=(const stored_service_reference & ref);
    stored_service_reference & operator=(stored_service_reference && ref) noexcept;

    ~stored_service_reference();

    explicit stored_service_reference(const service_reference & ref);

    explicit stored_service_reference(const type_id::type_id & type_id, const strings::string & str);

    explicit stored_service_reference(const type_id::type_id & type_id, strings::string && str);

    explicit stored_service_reference(const strings::string & str);

    explicit stored_service_reference(strings::string && str) noexcept;

    explicit stored_service_reference(const type_id::type_id & type_id) noexcept;

    J_INLINE_GETTER const type_id::type_id & service_type_id() const noexcept {
      return m_type_id;
    }

    J_INLINE_GETTER const strings::string & service_instance_name() const noexcept {
      return m_string;
    }

    bool operator==(const stored_service_reference & rhs) const noexcept;

    bool operator<(const stored_service_reference & rhs) const noexcept;

    bool empty() const noexcept {
      return m_type_id.empty() && m_string.empty();
    }

    template<typename Type, typename String>
    J_HIDDEN J_ALWAYS_INLINE bool matches(const Type & type_id, const String & service_instance_name) const noexcept {
      return (m_type_id.empty() || type_id == m_type_id)
        && (m_string.empty() || m_string == service_instance_name);
    }
  private:
    type_id::type_id m_type_id;
    strings::string  m_string;
  };
}
