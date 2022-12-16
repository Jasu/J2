#include "properties/access/string_access.hpp"
#include "strings/string.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::access {
  string_access_definition::string_access_definition() noexcept {
  }

  string_access_definition::~string_access_definition() {
  }

  void string_access_definition::initialize_string_access(
    get_size_t get_size,
    as_j_string_t as_j_string,
    as_j_string_view_t as_j_string_view,
    as_char_ptr_t as_char_ptr
  ) noexcept {
    m_get_size = get_size, m_as_j_string = as_j_string,
    m_as_j_string_view = as_j_string_view, m_as_char_ptr = as_char_ptr;
    J_ASSERT_NOT_NULL(m_get_size, m_as_j_string, m_as_j_string_view, m_as_char_ptr);
  }

  strings::string string_access::as_j_string() const {
    return static_cast<const string_access_definition &>(value_definition()).m_as_j_string(as_void_star());
  }

  strings::const_string_view string_access::as_j_string_view() const noexcept {
    return static_cast<const string_access_definition &>(value_definition()).m_as_j_string_view(as_void_star());
  }
}
