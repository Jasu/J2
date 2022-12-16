#include "services/configuration/configuration_union.hpp"
#include "hzd/crc32.hpp"
#include "properties/access/registry.hpp"
#include "containers/hash_map.hpp"
#include "properties/visiting/visit_recursive.hpp"
#include "properties/access_by_path.hpp"
#include "properties/path_hash.hpp"
#include "strings/string_hash.hpp"
#include "containers/vector.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "containers/unsorted_string_map.hpp"

namespace j::services::configuration {
  namespace {
    using path_map_t = hash_map<properties::path, int, properties::path_hash>;

    void visit_overlay(properties::access::typed_access & overlay_value,
                       const properties::visiting::visit_path & path_,
                       const configuration_overlay::paths_t & overlay_paths,
                       int overlay_priority,
                       path_map_t & configured_paths,
                       properties::wrappers::wrapper & object)
    {
      properties::path path(path_);
      if (!overlay_paths.contains(path)) {
        return;
      }

      auto it = configured_paths.find(path);
      if (it != configured_paths.end()) {
        if (it->second > overlay_priority) {
          return;
        }
        it->second = overlay_priority;
      } else {
        configured_paths.emplace(path, overlay_priority);
      }

      using properties::operator|;
      auto slot = properties::access_by_path(
        object,
        path,
        properties::access_by_path_flag::create
        | properties::access_by_path_flag::properties_as_keys);
      slot.set_value(overlay_value);
    }
  }

  configuration_union::configuration_union() noexcept {
  }

  configuration_union::configuration_union(configuration_union && rhs) noexcept
    : m_configuration_definition(rhs.m_configuration_definition),
      m_overlays(static_cast<vector<configuration_overlay> &&>(rhs.m_overlays)),
      m_factory(static_cast<config_factory_t &&>(rhs.m_factory))
  {
  }


  configuration_union & configuration_union::operator=(configuration_union && rhs) noexcept {
    if (this != &rhs) {
      m_configuration_definition = rhs.m_configuration_definition;
      m_overlays = static_cast<vector<configuration_overlay> &&>(rhs.m_overlays);
      m_factory = static_cast<config_factory_t &&>(rhs.m_factory);
    }
    return *this;
  }

  configuration_union::configuration_union(const configuration_union & rhs)
    : m_configuration_definition(rhs.m_configuration_definition),
      m_overlays(rhs.m_overlays),
      m_factory(rhs.m_factory)
  {
  }

  configuration_union & configuration_union::operator=(const configuration_union & rhs) {
    if (this != &rhs) {
      m_configuration_definition = rhs.m_configuration_definition;
      m_overlays = rhs.m_overlays;
      m_factory = rhs.m_factory;
    }
    return *this;
  }

  configuration_union::configuration_union(const type_id::type_id & type, const config_factory_t & factory)
    : m_configuration_definition(
        properties::access::registry::get_wrapper_definition(type)
      ),
      m_factory(factory)
  {
    J_ASSERT_NOT_NULL(m_factory);
  }

  mem::shared_ptr<void> configuration_union::create_configuration() const {
    auto result = m_factory();
    J_ASSERT_NOT_NULL(result);
    properties::wrappers::wrapper result_wrapper{ *m_configuration_definition, result.get() };
    path_map_t map;
    for (auto & overlay : m_overlays) {
      auto object = overlay.configuration();
      properties::visiting::visit_recursive(object,
                                            visit_overlay,
                                            overlay.paths,
                                            overlay.priority(),
                                            map,
                                            result_wrapper);
    }
    return result;
  }

  mem::shared_ptr<void> configuration_union::create_default_configuration() const {
    return m_factory();
  }

  configuration_union::~configuration_union() {
  }
}
