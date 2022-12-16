#pragma once

#include "type_id/type_id.hpp"

namespace j::strings::inline parsing {
  class parser_base;

  void register_parser(const type_id::type_id & type, const parser_base * parser);
  void unregister_parser(const type_id::type_id & type);

  [[nodiscard]] const parser_base * maybe_get_parser(const type_id::type_id & type);
}
