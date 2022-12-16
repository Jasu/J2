#include "properties/access/registry.hpp"
#include "exceptions/assert_lite.hpp"
#include "properties/access/typed_access.hpp"
#include "util/singleton.hpp"
#include "properties/conversions/conversion.hpp"
#include "properties/conversions/conversion_definition.hpp"
#include "type_id/type_hash.hpp"
#include "containers/trivial_array.hpp"
#include "containers/vector.hpp"
#include "containers/hash_map.hpp"

namespace j::properties::access {
  namespace {
    struct pending_record {
      enum class TYPE {
        WRAPPER,
        ACCESS,
        CONVERSION,
      } type;
      union {
        registry::pending_wrapper_callback_t pending_wrapper_callback;
        registry::pending_access_callback_t pending_access_callback;
        conversions::conversion * conversion;
      };
      /// For conversions, this is also the source class of the conversion.
      type_id::type_id adder;
      /// User data passed to the callbacks.
      void * user_data;
    };

    struct type_record {
      typed_access_definition * access = nullptr;
      const wrappers::wrapper_definition * wrapper = nullptr;
      vector<pending_record> pending;
    };

    using registry_t = hash_map<type_id::type_id, type_record, type_id::hash>;
    static util::singleton<registry_t> g_registry;
  }

  void registry::add_type(type_id::type_id type,
                          typed_access_definition * access,
                          const wrappers::wrapper_definition * wrapper)
  {
    J_ASSERT_NOT_NULL(access, wrapper);
    auto p = g_registry->emplace(type, type_record{});
    auto & record = p.first->second;
    J_ASSERT(!record.access, "Tried to register a typed_access twice.");
    J_ASSERT(!record.wrapper, "Tried to register a wrapper twice.");
    record.access = access;
    record.wrapper = wrapper;
    for (auto & it : record.pending) {
      switch (it.type) {
      case pending_record::TYPE::ACCESS:
        it.pending_access_callback(access, it.user_data);
        break;
      case pending_record::TYPE::CONVERSION:
        access->conversions().add_conversion_from(it.adder, *it.conversion);
        ::delete it.conversion;
        break;
      case pending_record::TYPE::WRAPPER:
        it.pending_wrapper_callback(wrapper, it.user_data);
      }
    }
    record.pending.clear();
  }

  void registry::remove_type(type_id::type_id type, const trivial_array<type_id::type_id> & conversions) noexcept {
    if (!g_registry.is_initialized) {
      return;
    }
    auto registry = g_registry.unsafe_instance();

    for (auto & t : conversions) {
      auto it = registry->find(t);
      if (it == registry->end()) {
        // This is ok, types and conversions need not be added or removed in order.
        continue;
      }
      if (it->second.access) {
        it->second.access->conversions().remove_conversion_from(type);
      } else {
        for (auto it2 = it->second.pending.begin(); it2 != it->second.pending.end(); ++it2) {
          if (it2->adder == type && it2->type == pending_record::TYPE::CONVERSION) {
            delete it2->conversion;
            it->second.pending.erase(it2);
            break;
          }
        }
      }
    }

    auto it = registry->find(type);
    J_ASSERT(it != registry->end(), "Tried to remove a non-existent access.");
    registry->erase(it);
    for (auto & rec : *registry) {
      for (auto it = rec.second.pending.begin(); it != rec.second.pending.end();) {
        // Check the record type - wrapper records are removed when the wrapper is removed.
        if (it->adder != type) {
          ++it;
          continue;
        }
        if (it->type == pending_record::TYPE::CONVERSION) {
          delete it->conversion;
        }
        it = rec.second.pending.erase(it);
      }
    }
  }

  void registry::add_wrapper(const std::type_info & type,
                             const wrappers::wrapper_definition * wrapper)
  {
    J_ASSERT_NOT_NULL(wrapper);
    auto p = g_registry->emplace(type_id::type_id(type), type_record{});
    auto & record = p.first->second;
    J_ASSERT(!record.wrapper, "Tried to register a wrapper twice.");
    record.wrapper = wrapper;
    for (auto it = record.pending.begin(); it != record.pending.end();) {
      if (it->type == pending_record::TYPE::WRAPPER) {
        it->pending_wrapper_callback(wrapper, it->user_data);
        it = record.pending.erase(it);
      } else {
        ++it;
      }
    }
  }

  void registry::remove_wrapper(const std::type_info & type_) noexcept {
    if (!g_registry.is_initialized) {
      return;
    }
    type_id::type_id type(type_);
    auto registry = g_registry.unsafe_instance();
    auto it = registry->find(type);
    J_ASSERT(it != registry->end() && it->second.wrapper,
             "Tried to remove a non-existent wrapper.");
    it->second.wrapper = nullptr;
    if (!it->second.access) {
      registry->erase(it);
    }

    for (auto & rec : *registry) {
      for (auto it = rec.second.pending.begin(); it != rec.second.pending.end();) {
        // Check the record type - access records are removed when the access is removed.
        if (it->adder == type && it->type == pending_record::TYPE::WRAPPER) {
          it = rec.second.pending.erase(it);
        } else {
          ++it;
        }
      }
    }
  }

  type_id::type_id registry::add_conversion_to(const conversions::conversion_definition & conversion) {
    J_ASSERT(conversion.from != conversion.to, "Tried to add an identity conversion.");
    auto p = g_registry->emplace(conversion.to, type_record{});
    auto & target = p.first->second;
    if (target.access) {
      target.access->conversions().add_conversion_from(conversion.from, conversion.conversion);
    } else {
      target.pending.emplace_back(pending_record{
        .type = pending_record::TYPE::CONVERSION,
        .conversion = ::new conversions::conversion(conversion.conversion),
        .adder = conversion.from,
      });
    }
    return conversion.to;
  }

  const wrappers::wrapper_definition * registry::get_wrapper_definition(const type_id::type_id & type) {
    auto & rec = g_registry->at(type);
    J_ASSERT_NOT_NULL(rec.wrapper);
    return rec.wrapper;
  }

  const wrappers::wrapper_definition * registry::get_wrapper_definition(const std::type_info & type) {
    return get_wrapper_definition(type_id::type_id(type));
  }

  const typed_access_definition * registry::get_typed_access_definition(const std::type_info & type) {
    auto & rec = g_registry->at(type_id::type_id{type});
    J_ASSERT_NOT_NULL(rec.access);
    return rec.access;
  }

  void registry::wait_for_wrapper(type_id::type_id adder,
                                  type_id::type_id wrapper_type,
                                  pending_wrapper_callback_t fn,
                                  void * user_data)
  {
    J_ASSUME_NOT_NULL(fn);
    auto p = g_registry->emplace(wrapper_type, type_record{});
    auto & rec = p.first->second;
    if (rec.wrapper) {
      fn(rec.wrapper, user_data);
    } else {
      rec.pending.emplace_back(pending_record{
        .type = pending_record::TYPE::WRAPPER,
        .pending_wrapper_callback = fn,
        .adder = adder,
        .user_data = user_data,
      });
    }
  }

  void registry::wait_for_access(type_id::type_id adder,
                                 type_id::type_id wrapper_type,
                                 pending_access_callback_t fn,
                                 void * user_data)
  {
    J_ASSUME_NOT_NULL(fn);
    auto p = g_registry->emplace(wrapper_type, type_record{});
    auto & rec = p.first->second;
    if (rec.access) {
      fn(rec.access, user_data);
    } else {
      rec.pending.emplace_back(pending_record{
        .type = pending_record::TYPE::ACCESS,
        .pending_access_callback = fn,
        .adder = adder,
        .user_data = user_data,
      });
    }
  }
}
