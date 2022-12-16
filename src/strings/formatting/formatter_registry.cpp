#include "strings/formatting/formatter_registry.hpp"

#include "strings/string.hpp"
#include "type_id/type_hash.hpp"
#include "strings/string_hash.hpp"
#include "containers/hash_map.hpp"
#include "util/singleton.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings {
  inline namespace formatting {
    namespace {
      struct J_TYPE_HIDDEN formatter_key {
        type_id::type_id type;
        strings::const_string_view name;
        J_A(AI,NODISC) inline bool operator==(const formatter_key &) const noexcept = default;
      };

      struct J_TYPE_HIDDEN formatter_key_hash final {
        inline u32_t operator()(const formatter_key & a) const noexcept {
          return crc32(type_id_hash(a.type), a.name.begin(), a.name.size());
        }
        const inline static type_id::hash type_id_hash;
      };

      struct J_TYPE_HIDDEN registry_data {
        hash_map<type_id::type_id, const formatter *, type_id::hash> default_formatters;
        hash_map<formatter_key, const formatter *, formatter_key_hash> named_formatters;
      };

      J_A(NODESTROY) constinit util::singleton<registry_data> g_formatters;
    }

    void register_formatter(
      type_id::type_id type,
      const char * name,
      bool is_default,
      const formatter * formatter
    ) {
      J_ASSERT(!type.empty(), "Tried to register a formatter for an empty type.");
      J_ASSERT_NOT_NULL(formatter);
      if (name) {
        [[maybe_unused]] bool did_insert = g_formatters->named_formatters.emplace(formatter_key{type, name}, formatter).second;
        J_ASSERT(did_insert, "Tried to register a formatter twice.");
      }
      if (is_default) {
        [[maybe_unused]] bool did_insert = g_formatters->default_formatters.emplace(type, formatter).second;
        J_ASSERT(did_insert, "Tried to register a formatter twice.");
      }
    }

    void unregister_formatter(
      type_id::type_id type,
      const char * name,
      bool is_default
    ) {
      if (!g_formatters.is_initialized) {
        return;
      }
      if (name) {
        auto & m = g_formatters.unsafe_instance()->named_formatters;
        m.erase(m.find(formatter_key{type, name}));
      }
      if (is_default) {
        auto & m = g_formatters.unsafe_instance()->default_formatters;
        m.erase(m.find(type));
      }
    }

    const formatter * do_maybe_get_formatter(type_id::type_id type) {
      if (!g_formatters.is_initialized) {
        return nullptr;
      }
      auto ptr = g_formatters.unsafe_instance()->default_formatters.maybe_at(type);
      return ptr ? *ptr : nullptr;
    }

    const formatter * do_maybe_get_formatter(type_id::type_id type, const_string_view name) {
      if (!g_formatters.is_initialized) {
        return nullptr;
      }
      auto ptr = g_formatters.unsafe_instance()->named_formatters.maybe_at({type, name});
      return ptr ? *ptr : nullptr;
    }
  }
}
