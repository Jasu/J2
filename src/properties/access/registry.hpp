#pragma once

#include "type_id/type_id.hpp"

#include "hzd/type_traits.hpp"

#include "properties/access/default_accesses.hpp"

namespace j::inline containers {
  template<typename T> class trivial_array;
}

namespace j::properties::conversions {
  class conversion_definition;
}

namespace j::properties::access {
  class typed_access_definition;
  namespace registry {
    namespace detail {
      template<typename T>
      struct J_TYPE_HIDDEN array_to_unbounded {
        using type J_NO_DEBUG_TYPE = T;
      };

      template<typename T, auto Sz>
      struct J_TYPE_HIDDEN array_to_unbounded<T[Sz]> {
        using type J_NO_DEBUG_TYPE = T[];
      };

      template<typename T>
      using normalize_type_t J_NO_DEBUG_TYPE = typename array_to_unbounded<j::remove_cref_t<T>>::type;
    }

    using pending_wrapper_callback_t J_NO_DEBUG_TYPE = void (*)(const wrappers::wrapper_definition *, void *);
    using pending_access_callback_t J_NO_DEBUG_TYPE = void (*)(const typed_access_definition *, void *);

    /// Add a typed access definition and its reference wrapper to the registry.
    ///
    /// The access must not be registered.
    void add_type(
      type_id::type_id type,
      typed_access_definition * access,
      const wrappers::wrapper_definition * wrapper);

    /// Remove a typed access definition and its reference wrapper from the registry.
    ///
    /// The access must be registered.
    void remove_type(type_id::type_id type, const trivial_array<type_id::type_id> & conversions) noexcept;

    /// Add a wrapper definition to the registry.
    ///
    /// The wrapper must not be registered.
    void add_wrapper(const std::type_info & type, const wrappers::wrapper_definition * wrapper);

    /// Remove a wrapper from the registry.
    ///
    /// The wrapper must be registered.
    void remove_wrapper(const std::type_info & type) noexcept;

    type_id::type_id add_conversion_to(const conversions::conversion_definition & conversion);

    /// Get the [wrappers::wrapper_definition] of type or throw.
    const wrappers::wrapper_definition * get_wrapper_definition(const type_id::type_id & type);

    /// Get the [wrappers::wrapper_definition] of type or throw.
    const wrappers::wrapper_definition * get_wrapper_definition(const std::type_info & type);

    /// Get the [typed_access_definition] of type or throw.
    const typed_access_definition * get_typed_access_definition(const std::type_info & type);

    template<typename T>
    J_INLINE_GETTER static const wrappers::wrapper_definition * get_wrapper_definition() {
      using TNorm J_NO_DEBUG_TYPE = detail::normalize_type_t<T>;
      if constexpr (has_default_access_v<TNorm>) {
        return wrapper_definition_v<TNorm>;
      } else {
        return get_wrapper_definition(typeid(TNorm));
      }
    }

    template<typename T>
    J_INLINE_GETTER static const typed_access_definition * get_typed_access_definition() {
      using TNorm J_NO_DEBUG_TYPE = detail::normalize_type_t<T>;
      if constexpr (has_default_access_v<TNorm>) {
        return typed_access_definition_v<TNorm>;
      } else {
        return get_typed_access_definition(typeid(TNorm));
      }
    }

    /// Call [fn] when the [wrappers::wrapper_definition] of [wrapper_type] is registered.
    ///
    /// \param adder specifies the class that is waiting for the wrapper. If the class is removed
    ///              from the registry, its wait will be removed as well.
    void wait_for_wrapper(type_id::type_id adder,
                          type_id::type_id wrapper_type,
                          pending_wrapper_callback_t fn,
                          void * user_data);

    /// Call [fn] when the [typed_access_definition] of [wrapper_type] is registered.
    ///
    /// \param adder specifies the class that is waiting for the wrapper. If the class is removed
    ///              from the registry, its wait will be removed as well.
    void wait_for_access(type_id::type_id adder,
                         type_id::type_id wrapper_type,
                         pending_access_callback_t fn,
                         void * user_data);
  }
}
