#pragma once

#include "services/registry/registry.hpp"
#include "services/metadata.hpp"

namespace j::services {

  template<typename Service>
  class J_HIDDEN service_instance final {
    const class_metadata m_metadata;
  public:
    template<typename... Args>
    J_ALWAYS_INLINE explicit service_instance(const char * J_NOT_NULL name, const char * J_NOT_NULL title, Args && ... args)
      : m_metadata(name, title, static_cast<Args &&>(args)...)
    {
      registry::instance->register_instance(typeid(Service), &m_metadata);
    }
  };
}
