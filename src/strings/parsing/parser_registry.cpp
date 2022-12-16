#include "strings/parsing/parser_registry.hpp"

#include "containers/hash_map.hpp"
#include "type_id/type_hash.hpp"
#include "util/singleton.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings::inline parsing {
  namespace {
    J_A(NODESTROY) constinit util::singleton<hash_map<type_id::type_id, const parser_base *, type_id::hash>> g_parsers;
  }

  void register_parser(const type_id::type_id & type, const parser_base * parser) {
    J_ASSERT(!type.empty(), "Tried to register a parser for an empty type.");
    J_ASSERT_NOT_NULL(parser);
    [[maybe_unused]] bool did_insert = g_parsers->emplace(type, parser).second;
    J_ASSERT(did_insert, "Tried to register a parser twice.");
  }

  void unregister_parser(const type_id::type_id & type) {
    J_ASSERT(!type.empty(), "Tried to unregister a parser for an empty type.");
    if (!g_parsers.is_initialized) {
      return;
    }
    auto inst = g_parsers.unsafe_instance();
    inst->erase(inst->find(type));
  }

  [[nodiscard]] const parser_base * maybe_get_parser(const type_id::type_id & type) {
    if (!g_parsers.is_initialized) {
      return nullptr;
    }
    auto it = g_parsers.unsafe_instance()->maybe_at(type);
    return it ? *it : nullptr;
  }
}
