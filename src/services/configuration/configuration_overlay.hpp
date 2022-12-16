#pragma once

#include "containers/hash_map_fwd.hpp"
#include "properties/wrappers/stored_wrapper.hpp"
#include "properties/path_hash.hpp"

J_DECLARE_EXTERN_HASH_SET(j::properties::path, HASH(j::properties::path_hash), EMPLACE(const j::properties::path &));

namespace j::services::configuration {
  /// Configuration parsed by a single configuration provider for a single configuration object.
  class configuration_overlay {
  public:
    using paths_t J_NO_DEBUG_TYPE = hash_set<properties::path, properties::path_hash>;
    configuration_overlay() noexcept;

    configuration_overlay(const properties::wrappers::stored_wrapper & configuration, int priority);

    configuration_overlay(properties::wrappers::stored_wrapper && configuration, int priority);

    configuration_overlay(configuration_overlay && rhs) noexcept;
    configuration_overlay(const configuration_overlay & rhs);
    configuration_overlay & operator=(configuration_overlay && rhs) noexcept;
    configuration_overlay & operator=(const configuration_overlay & rhs);
    ~configuration_overlay();

    template<typename Paths, typename Configuration>
    configuration_overlay(Paths && paths, Configuration && configuration, int priority) noexcept
      : paths(static_cast<Paths &&>(paths)),
        m_configuration(static_cast<Configuration &&>(configuration)),
        m_priority(priority)
    {
    }

    J_INLINE_GETTER const properties::wrappers::wrapper & configuration() const noexcept {
      return m_configuration.wrapper();
    }

    J_INLINE_GETTER int priority() const noexcept {
      return m_priority;
    }

    bool contains(const properties::path & p) const noexcept;

    /// Set of paths defined in this overlay.
    paths_t paths;
  private:
    properties::wrappers::stored_wrapper m_configuration;
    /// Priority of this configuration overlay.
    ///
    /// Higher number means higher priority.
    int m_priority;
  };
}
