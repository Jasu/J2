#pragma once

#include "containers/vector.hpp"
#include "functions/bound_function.hpp"

namespace j::type_id {
  struct type_id;
}

namespace j::mem {
  template<typename> class shared_ptr;
}

namespace j::services::configuration {
  class configuration_overlay;
}


namespace j::properties::wrappers {
  class wrapper_definition;
}

namespace j::services::configuration {
  /// Set of configuration overlays, describing the configuration of a service.
  using config_factory_t = functions::bound_function<mem::shared_ptr<void> ()>;

  class configuration_union final {
  public:
    configuration_union() noexcept;

    configuration_union(configuration_union && rhs) noexcept;
    configuration_union & operator=(configuration_union && rhs) noexcept;
    configuration_union(const configuration_union & rhs);
    configuration_union & operator=(const configuration_union & rhs);

    configuration_union(const type_id::type_id & type, const config_factory_t & factory);

    ~configuration_union();

    template<typename... Args>
    void add_overlay(Args && ... args) {
      m_overlays.emplace_back(static_cast<Args &&>(args)...);
    }

    mem::shared_ptr<void> create_configuration() const;

    mem::shared_ptr<void> create_default_configuration() const;

    const properties::wrappers::wrapper_definition & definition() const noexcept {
      return *m_configuration_definition;
    }
  private:
    const properties::wrappers::wrapper_definition * m_configuration_definition;
    vector<configuration_overlay> m_overlays;
    config_factory_t m_factory;
  };
}
