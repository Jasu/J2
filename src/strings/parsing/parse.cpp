#include "strings/parsing/parse.hpp"
#include "exceptions/assert_lite.hpp"
#include "strings/parsing/parser_registry.hpp"
#include "strings/parsing/parser.hpp"

namespace j::strings {
  inline namespace parsing {
    mem::any parse(type_id::type_id type, const strings::const_string_view & str) {
      auto p = maybe_get_parser(type);
      J_ASSERT_NOT_NULL(p);
      return p->parse(str);
    }
  }
}
