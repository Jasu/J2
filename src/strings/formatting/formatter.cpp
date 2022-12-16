#include "strings/formatting/formatter_unknown_length.hpp"

#include "strings/formatting/formatter_registry.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings::inline formatting {
  format_value::format_value(const strings::string & value) noexcept
    : m_type(typeid(strings::const_string_view), (u8_t)inline_type::string),
      m_value{ .sv = { value.data(), value.size() } }
  { }

  formatter::formatter(bool supports_get_length,
                       u32_t num_types,
                       type_id::type_id * J_NOT_NULL types,
                       const char * name,
                       bool is_default) noexcept
    : supports_get_length(supports_get_length),
      m_is_default(is_default),
      m_num_types(num_types),
      m_types(types),
      m_name(name)
  {
    for (u32_t i = 0; i < num_types; ++i) {
      register_formatter(m_types[i], m_name, is_default, this);
    }
  }

  formatter::formatter(bool supports_get_length,
                       const std::type_info & type,
                       const char * name,
                       bool is_default) noexcept
    : supports_get_length(supports_get_length),
      m_is_default(is_default),
      m_num_types(1),
      m_type(type),
      m_name(name)
  {
    register_formatter(m_type, m_name, is_default, this);
  }
  void formatter::format(const const_string_view &,
                         const void *,
                         styled_sink &, style) const {
    J_ASSERT(!supports_get_length,
             "m_supports_get_length is set, but formatting with length is not overridden.");
    J_THROW("Get length is not supported.");
  }

  u32_t formatter::get_length(const const_string_view &,
                              const void *) const noexcept {
    J_ASSERT(!supports_get_length,
             "m_supports_get_length is set, but getting length is not overridden.");
    J_THROW("Get length is not supported.");
  }

  styled_string formatter::format(const const_string_view &,
                                  const void *) const {
    J_ASSERT(supports_get_length,
             "m_supports_get_length is not set, but formatting without length is not overridden.");
    J_THROW("Non-get-length operation is not supported.");
  }

  formatter::~formatter() {
    if (m_num_types == 1) {
      unregister_formatter(m_type, m_name, m_is_default);
    } else {
      for (u32_t i = 0; i < m_num_types; ++i) {
        unregister_formatter(m_types[i], m_name, m_is_default);
      }
      ::j::free(const_cast<type_id::type_id*>(m_types));
    }
  }
}
