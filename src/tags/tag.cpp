#include "tags/tag.hpp"
#include "strings/string.hpp"

namespace j::tags {
  tag::tag(const tag_definition_base * definition, mem::any && value) noexcept
    : m_definition(definition),
      m_value(static_cast<mem::any &&>(value))
  {
    J_ASSERT_NOT_NULL(m_definition);
    J_ASSERT_NOT_NULL(m_definition->format_string, *m_definition->format_string);
  }

  tag::tag(tag && rhs) noexcept
    : m_definition(rhs.m_definition),
      m_value(static_cast<mem::any &&>(rhs.m_value))
  {
  }

  tag & tag::operator=(tag && rhs) noexcept {
    m_definition = rhs.m_definition;
    m_value = static_cast<mem::any &&>(rhs.m_value);
    return *this;
  }

  tag::~tag() {
  }

  tag tag_definition<strings::string>::operator()(const strings::string & value) const {
    return tag{this, mem::any(value)};
  }

  tag tag_definition<strings::string>::operator()(strings::string && value) const {
    return tag{this, mem::any(static_cast<strings::string &&>(value))};
  }

  tag tag_definition<strings::string>::operator()(const char * value) const {
    return tag{this, mem::any(strings::string(value))};
  }

  tag tag_definition<strings::string>::operator()(const strings::const_string_view & value) const {
    return tag{this, mem::any(strings::string(value))};
  }
}

template class j::tags::tag_definition<j::strings::string>;
