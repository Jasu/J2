#include "mem/any.hpp"
#include "strings/string.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::mem {
  template void any::initialize<strings::const_string_view, const strings::const_string_view &>(const strings::const_string_view &);
  template void any::initialize<strings::const_string_view, strings::const_string_view>(strings::const_string_view &&);

  template void any::initialize<strings::string, const strings::string &>(const strings::string &);
  template void any::initialize<strings::string, strings::string>(strings::string &&);

  namespace detail {
    [[noreturn]] void throw_invalid_type() {
      J_THROW("Invalid type in mem::any.");
    }
  }
  any & any::operator=(any && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      if (m_deleter && m_type.get_tag_bit(0)) {
        m_deleter(m_value);
      }
      m_value = rhs.m_value;
      m_deleter = rhs.m_deleter;
      m_type = rhs.m_type;
      rhs.m_value = nullptr;
      rhs.m_deleter = nullptr;
      rhs.m_type.reset();
    }
    return *this;
  }
}
