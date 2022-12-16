#include "services/configuration/configuration_overlay.hpp"
#include "properties/path_cmp.hpp"
#include "properties/visiting/visit_recursive.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_SET(j::properties::path, HASH(j::properties::path_hash), EMPLACE(const j::properties::path &));

namespace j::services::configuration {
  namespace {
    void visitor(const properties::access::typed_access &,
                 const properties::visiting::visit_path & path,
                 hash_set<properties::path, properties::path_hash> & paths)
    {
      paths.emplace(properties::path(path));
    }
  }

  configuration_overlay::configuration_overlay() noexcept {
  }

  configuration_overlay::configuration_overlay(const properties::wrappers::stored_wrapper & configuration, int priority)
    : m_configuration(configuration),
      m_priority(priority)
  {
    auto w = configuration.wrapper();
    properties::visiting::visit_recursive(w, &visitor, paths);
  }

  configuration_overlay::configuration_overlay(properties::wrappers::stored_wrapper && configuration, int priority)
    : m_configuration(static_cast<properties::wrappers::stored_wrapper &&>(configuration)),
      m_priority(priority)
  {
    properties::visiting::visit_recursive(configuration.wrapper(), &visitor, paths);
  }

  configuration_overlay::configuration_overlay(configuration_overlay && rhs) noexcept
    : paths(static_cast<paths_t &&>(rhs.paths)),
      m_configuration(static_cast<properties::wrappers::stored_wrapper &&>(rhs.m_configuration)),
      m_priority(rhs.m_priority)
  {
  }

  configuration_overlay::configuration_overlay(const configuration_overlay & rhs)
    : paths(rhs.paths),
      m_configuration(rhs.m_configuration),
      m_priority(rhs.m_priority)
  {
  }

  configuration_overlay & configuration_overlay::operator=(configuration_overlay && rhs) noexcept {
    if (this != &rhs) {
      paths = static_cast<paths_t &&>(rhs.paths);
      m_configuration = static_cast<properties::wrappers::stored_wrapper &&>(rhs.m_configuration);
      m_priority = rhs.m_priority;
    }
    return *this;
  }

  configuration_overlay & configuration_overlay::operator=(const configuration_overlay & rhs) {
    if (this != &rhs) {
      paths = rhs.paths;
      m_configuration = rhs.m_configuration;
      m_priority = rhs.m_priority;
    }
    return *this;
  }

  bool configuration_overlay::contains(const properties::path & p) const noexcept {
    return paths.contains(p);
  }

  configuration_overlay::~configuration_overlay() {
  }
}
