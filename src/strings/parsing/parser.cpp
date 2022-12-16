#include "strings/parsing/parser.hpp"
#include "strings/parsing/parser_registry.hpp"

namespace j::strings {
  inline namespace parsing {
    parser_base::parser_base(const std::type_info & type) noexcept
      : m_type(type)
    {
      register_parser(m_type, this);
    }

    parser_base::~parser_base() {
      unregister_parser(m_type);
    }
  }
}
