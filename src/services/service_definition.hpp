#pragma once

#include "services/registry/registry.hpp"
#include "services/factory.hpp"
#include "services/injected_calls/type_erased_injected_call.hpp"
#include "services/finalizer_call.hpp"
#include "services/metadata.hpp"
#include "services/detail/setting.hpp"
#include "services/detail/construction_handler.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/foreach.hpp"
#include "attributes/trivial_array.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::services::detail::setting);
J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::services::registry::implements_record);
J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::services::injected_calls::type_erased_injected_call);

namespace j::services {
  namespace detail {
    template<typename Attr>
    [[maybe_unused]] inline static constexpr bool has_initializer_dep_v = false;

    template<typename T>
    [[maybe_unused]] inline static constexpr bool has_initializer_dep_v<attributes::attribute<attributes::attribute_definition<mem::any, initialize_tag, attributes::multiple, void>, T> &> = T::has_dependencies_v;

    template<sz_t NumDeps, typename... Args>
    [[maybe_unused]] inline static constexpr bool has_initializer_deps_v = (has_initializer_dep_v<Args &> || ...);

    template<typename... Args>
    [[maybe_unused]] inline static constexpr bool has_initializer_deps_v<0, Args...> = false;
  }

  class service_definition_base {
  protected:
    trivial_array<detail::setting> m_settings;
    trivial_array<injected_calls::type_erased_injected_call> m_initializers;
    trivial_array<registry::implements_record> m_implements;
    detail::finalizer_call_set m_finalizers;
    type_id::type_id m_type;
    const bool m_was_interface_registered;

    service_definition_base(sz_t num_settings,
                            sz_t num_initializers,
                            sz_t num_implements,
                            sz_t num_finalizers,
                            bool was_interface_registered,
                            const std::type_info & type);

  public:
    ~service_definition_base();

    service_definition_base(const service_definition_base &) = delete;
  };

  template<class Service>
  class J_TYPE_HIDDEN service_definition : public service_definition_base {
    const class_metadata m_metadata;
  public:
    template<
      typename... Args,
      sz_t NumSettings = ::j::attributes::count<Args...>(setting),
      sz_t NumImplements = ::j::attributes::count<Args...>(implements),
      sz_t NumInitializers = ::j::attributes::count<Args...>(initialize),
      sz_t NumFinalizers = ::j::attributes::count<Args...>(finalize)
    >
    J_ALWAYS_INLINE_NO_DEBUG explicit service_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL title, Args && ... metadata)
      : service_definition_base(
          NumSettings,
          NumInitializers,
          NumImplements,
          NumFinalizers,
          !j::attributes::has<Args...>(no_interface),
          typeid(Service)
        ),
      m_metadata(name, title, static_cast<Args &&>(metadata)...)
    {
      if constexpr (NumSettings != 0) {
        j::attributes::populate_trivial_array_move(
          m_settings, setting, static_cast<Args &&>(metadata)...);
      }
      if constexpr (NumInitializers != 0) {
        j::attributes::populate_trivial_array_move(
          m_initializers, initialize, static_cast<Args &&>(metadata)...);
      }
      if constexpr (NumFinalizers != 0) {
        j::attributes::foreach_member_noexcept(finalize, m_finalizers,
                                      &detail::finalizer_call_set::add_finalizer<Service>, static_cast<Args &&>(metadata)...);
      }
      initialization_stage initialization_stage = initialization_stage::lazy;
      if constexpr (j::attributes::has<Args...>(initialize_by_default)) {
        initialization_stage = initialize_by_default.get(static_cast<Args &&>(metadata)...);
      }

      if constexpr (NumImplements > 0) {
        j::attributes::foreach_noexcept(implements, [this](const auto & v) noexcept {
          m_implements.initialize_element(registry::make_implements_record<Service>(v));
        }, static_cast<Args &&>(metadata)...);
      }

      if constexpr (!j::attributes::has<Args...>(create)) {
        if constexpr (j::attributes::has<Args...>(global_singleton)) {
          // Global singleton, created with placement new
          using ctor_t J_NO_DEBUG_TYPE = detail::placement_singleton_construction_handler<
            Service,
            injected_calls::wired_argument_list<>,
            j::attributes::has<Args...>(no_delete),
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers
          >;
          ctor_t::initialize(injected_calls::wired_argument_list<>{}, m_initializers, m_finalizers);
          registry::instance->register_service(
            m_type,
            &m_metadata,
            &ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            &m_metadata,
            m_was_interface_registered,
            false);
        } else {
          // Default constructor
          using ctor_t J_NO_DEBUG_TYPE = detail::ctor_construction_handler<
            Service,
            injected_calls::wired_argument_list<>,
            j::attributes::has<Args...>(no_delete),
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers
          >;
          ctor_t::initialize(injected_calls::wired_argument_list<>{}, m_initializers, m_finalizers);
          registry::instance->register_service(
            m_type,
            &m_metadata,
            &ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            j::attributes::has<Args...>(no_default_instance) ? nullptr : &m_metadata,
            m_was_interface_registered,
            j::attributes::has<Args...>(one_per_service));
          }
      } else {
        auto factory_object = create.get(static_cast<Args &&>(metadata)...);
        if constexpr (is_ptr_v<decltype(factory_object)>) {
          // Direct pointer to the service, i.e. a static instance.
          using ctor_t J_NO_DEBUG_TYPE = detail::static_singleton_construction_handler<
            Service,
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers>;

          ctor_t::initialize(factory_object, m_initializers, m_finalizers);

          registry::instance->register_service(
            m_type,
            &m_metadata,
            ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            &m_metadata,
            m_was_interface_registered,
            false);
        } else if constexpr (injected_calls::is_injected_call_v<decltype(factory_object)>) {
          // Factory function
          using ctor_t J_NO_DEBUG_TYPE = detail::factory_construction_handler<
            Service,
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers,
            decltype(factory_object),
            j::attributes::has<Args...>(no_delete)
          >;

          ctor_t::initialize(static_cast<decltype(factory_object) &&>(factory_object), m_initializers, m_finalizers);

          registry::instance->register_service(
            m_type,
            &m_metadata,
            ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            j::attributes::has<Args...>(no_default_instance) ? nullptr : &m_metadata,
            m_was_interface_registered,
            false);
        } else if constexpr (j::attributes::has<Args...>(global_singleton)) {
          // Global singleton, created with placement new
          using ctor_t J_NO_DEBUG_TYPE = detail::placement_singleton_construction_handler<
            Service,
            decltype(factory_object),
            j::attributes::has<Args...>(no_delete),
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers
          >;
          ctor_t::initialize(static_cast<decltype(factory_object) &&>(factory_object), m_initializers, m_finalizers);
          registry::instance->register_service(
            m_type,
            &m_metadata,
            ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            &m_metadata,
            m_was_interface_registered,
            false);
        } else {
          using ctor_t J_NO_DEBUG_TYPE = detail::ctor_construction_handler<
            Service,
            decltype(factory_object),
            j::attributes::has<Args...>(no_delete),
            NumInitializers,
            detail::has_initializer_deps_v<NumInitializers, Args...>,
            NumFinalizers
          >;
          ctor_t::initialize(static_cast<decltype(factory_object) &&>(factory_object), m_initializers, m_finalizers);
          registry::instance->register_service(
            m_type,
            &m_metadata,
            &ctor_t::construct,
            &m_settings,
            &m_implements,
            initialization_stage,
            j::attributes::has<Args...>(no_default_instance) ? nullptr : &m_metadata,
            m_was_interface_registered,
            j::attributes::has<Args...>(one_per_service));
        }
      }
    }
  };
}
