#pragma once

#include "properties/wrappers/wrapper_definition.hpp"
#include "properties/access/typed_access.hpp"
#include "properties/detail/value_definition_impl.hpp"
#include "properties/wrappers/wrapper.hpp"

namespace j::properties::wrappers {
  namespace detail {
    wrapper create_reference_wrapper(const wrapper_definition & def, access::typed_access & a);
  }

  /// Wrapper capable of holding a reference of a single type.
  ///
  /// The wrapper cannot be empty.
  template<typename Type, auto TypedAccessPtr, PROPERTY_TYPE PropertyType>
  class J_TYPE_HIDDEN reference_wrapper_definition : public wrapper_definition {
    J_HIDDEN static void set_value(void * variant, const access::typed_access & a);

    J_HIDDEN static access::typed_access get_value(void * variant);

  public:
    J_HIDDEN J_ALWAYS_INLINE reference_wrapper_definition();
  };

  template<typename Type, auto TypedAccessPtr, PROPERTY_TYPE PropertyType>
  J_HIDDEN void reference_wrapper_definition<Type, TypedAccessPtr, PropertyType>::set_value(void * variant, const access::typed_access & a) {
    access::typed_access(*TypedAccessPtr, variant).assign_from(a);
  }

  template<typename Type, auto TypedAccessPtr, PROPERTY_TYPE PropertyType>
  J_HIDDEN access::typed_access reference_wrapper_definition<Type, TypedAccessPtr, PropertyType>::get_value(void * variant) {
    return {*TypedAccessPtr, variant};
  }

  template<typename Type, auto TypedAccessPtr, PROPERTY_TYPE PropertyType>
  J_HIDDEN J_ALWAYS_INLINE reference_wrapper_definition<Type, TypedAccessPtr, PropertyType>::reference_wrapper_definition()
    : wrapper_definition(
      properties::detail::make_value_definition<Type*>(),
      PropertyType,
      &set_value,
      &get_value,
      &detail::create_reference_wrapper,
      TypedAccessPtr)
    {
    }
}
