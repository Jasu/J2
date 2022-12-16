#pragma once

#include "properties/access/typed_access.hpp"
#include "properties/detail/value_definition_impl.hpp"

namespace j::properties::access {
  namespace detail {
    template<typename Type, typename Access, typename Enable = Type *>
    struct J_TYPE_HIDDEN create_default_wrapper {
      J_NO_DEBUG constexpr static auto create_default = nullptr;
    };

    template<typename Type, typename Access>
    struct J_TYPE_HIDDEN create_default_wrapper<Type, Access, decltype(::new Type)> {
      static void * create_default() {
        return ::new Type;
      }
    };
  }

  template<typename Type, typename Access>
  J_ALWAYS_INLINE void initialize_typed_access(
    typed_access_definition & def,
    PROPERTY_TYPE type,
    const wrappers::wrapper_definition * reference_wrapper,
    const char * type_name
  ) {
    def.initialize(
      properties::detail::make_value_definition<Type, Access>(),
      type,
      typeid(Type),
      type_name,
      reference_wrapper,
      detail::create_default_wrapper<Type, Access>::create_default
    );
  }
}
