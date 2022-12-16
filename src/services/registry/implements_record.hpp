#pragma once

#include "type_id/type_id.hpp"
#include "services/detail/implements.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::services::registry {
  /// Converter method for casting services to interfaces.
  using service_caster J_NO_DEBUG_TYPE = mem::shared_ptr<void>(*)(mem::shared_ptr<void>);

  template<typename From, typename To>
  J_NO_DEBUG mem::shared_ptr<void> cast(mem::shared_ptr<void> value) noexcept {
    return mem::shared_ptr<void>(
      static_cast<mem::shared_ptr<void> &&>(value),
      static_cast<To*>(reinterpret_cast<From*>(value.get())));
  }

  struct implements_record {
    type_id::type_id interface_type;
    service_caster cast;
    int default_priority;
  };

  template<typename Service, typename Interface>
  J_INLINE_GETTER implements_record make_implements_record(const detail::implements<Interface> & impl) noexcept {
    return implements_record{
      type_id::type_id(typeid(Interface)),
      &cast<Service, Interface>,
      impl.priority
    };
  }
}
