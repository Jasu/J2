#pragma once

#include "attributes/attributes.hpp"
#include "services/detail/setting.hpp"
#include "services/detail/implements.hpp"
#include "services/container/initialization_stage.hpp"
#include "services/injected_calls/injected_call.hpp"
#include "services/injected_calls/type_erased_injected_call.hpp"
#include "services/service_reference.hpp"
#include "services/registry/implements_record.hpp"

namespace j::services {
  using namespace attributes;

  using j::type;

  struct initialize_tag;
  /// Service policy specifying a class method to call when initializing the service.
  ///
  /// The method is called when the instance is created, not every time when a service requests the
  /// service. If that is desried, one_per_service should be specified as well.
  /// The function can take any arguments that are injectable from the container.
  ///
  /// Can be specified multiple times. The call order is the same as the specification order.
  // inline constexpr attributes::attribute_definition<attributes::any, struct initialize_tag, attributes::multiple> initialize;
  J_NO_DEBUG inline constexpr attribute_definition initialize(
    tag = type<initialize_tag>,
    is_multiple);

  /// Service policy specifying a class method to call when finalizing the service.
  ///
  /// The method is called when the instance is destroyed (all references returned). The function
  /// cannot take any arguments.
  ///
  /// Can be specified multiple times. The call order is the same as the specification order.
  J_NO_DEBUG inline constexpr attribute_definition finalize(
    tag = type<struct finalize_tag>,
    is_multiple);

  /// Service policy specifying that the service returned by the factory should not be deleted.
  ///
  /// Does not apply to factories returning mem::shared_ptr:s, or to global instances.
  // inline constexpr attributes::attribute_definition<bool, struct no_delete_tag, attributes::flag> no_delete;
  J_NO_DEBUG inline constexpr attribute_definition no_delete(
    tag = type<struct no_delete_tag>,
    is_flag);

  /// Service policy specifying that each service should have a separate instance of the interface.
  ///
  /// Used e.g. by the logger factory to create a seprate logging channel for each service.
  J_NO_DEBUG inline constexpr attribute_definition one_per_service(
    tag = type<struct one_per_service_tag>,
    is_flag);

  /// Service policy specifying that the service should not have a default instance.
  J_NO_DEBUG inline constexpr attribute_definition no_default_instance(
    tag = type<struct no_default_instance_tag>,
    is_flag);

  /// Service policy specifying that the service should be an application-wide singleton.
  ///
  /// The same singleton is shared among all containers (if multiple containers are used).
  J_NO_DEBUG inline constexpr attribute_definition global_singleton(
    tag = type<struct global_singleton_tag>,
    is_flag);

  /// Interface policy specifying that the interface is initialized during container initailzation.
  ///
  /// Used e.g. by the terminate_handler service to initialize the terminate handler without any
  /// other service explicitly requesting it.
  J_NO_DEBUG inline constexpr attribute_definition initialize_by_default(
    tag = type<struct initialize_by_default_tag>,
    value_type = type<initialization_stage>);

  /// Service policy specifying that the service cannot be injected directly.
  ///
  /// If specified, the service will not have itself registered as an interface. This ensures
  /// that the service can only be instatiated based on its other implemented interfaces.
  J_NO_DEBUG inline constexpr attribute_definition no_interface(
    tag = type<struct no_interface_tag>,
    is_flag);

  J_NO_DEBUG inline constexpr attribute_definition setting(
    tag = type<struct setting_tag>,
    value_type = type<detail::setting>,
    is_multiple);

  /// Interface policy specifying the default implementation.
  J_NO_DEBUG inline constexpr attribute_definition default_implementation(
    value_type = type<service_reference>,
    tag = type<struct default_implementation_tag>);

  template<typename Interface>
  J_NO_DEBUG inline constexpr const detail::implements<Interface> interface;


  /// Service policy specifying that the service implements an interface.
  ///
  /// Can be specified multiple times.
  J_NO_DEBUG inline constexpr attribute_definition implements(
    tag = type<struct implements_tag>,
    is_multiple);

  J_NO_DEBUG inline constexpr attribute_definition create(tag = type<struct create_tag>);

  template<typename Setting>
  [[nodiscard]] constexpr detail::setting setting_object(const char * name) noexcept {
    return {
      .name = name,
      .type_id = &type_id::get_type_id<Setting>,
      .create_configuration = &mem::make_void_shared<Setting>,
    };
  }

  constexpr const inline attributes::detail::without class_metadata_withouts_v = attributes::without(
            create,
            implements,
            initialize,
            finalize,
            global_singleton,
            no_delete,
            one_per_service,
            initialize_by_default,
            no_interface,
            setting,
            no_default_instance);

  /// Metadata about a single class, common to services and interfaces.
  struct class_metadata final {
    const char * J_AA(NOALIAS) const name;
    const char * J_AA(NOALIAS) const title;
    const attributes::attribute_map dynamic_metadata;

    J_BOILERPLATE(class_metadata, CTOR_DEL, COPY_DEL)

    explicit class_metadata(const char * J_NOT_NULL name, const char * J_NOT_NULL title) noexcept;

    template<typename... Args>
    J_NO_DEBUG J_HIDDEN explicit class_metadata(const char * name, const char * title, Args && ... args)
      : name(name),
        title(title),
        dynamic_metadata(class_metadata_withouts_v, static_cast<Args &&>(args)...)
    {
    }

    ~class_metadata();
  };

  template<typename Result, typename... Args, typename... ArgDefinitions>
  constexpr auto call(Result (*fn)(Args...), const ArgDefinitions & ... args) {
    return injected_calls::injected_call{
      injected_calls::required_tag_t::v,
      fn,
      injected_calls::wire<Args...>(args...)};
  }

  template<typename Result, typename Class, typename... Args, typename... ArgDefinitions>
  constexpr auto call(Result (Class::*fn)(Args...), const ArgDefinitions & ... args) {
    return injected_calls::injected_call{
      injected_calls::required_tag_t::v,
      fn,
      injected_calls::wire<Args...>(args...)};
  }

  template<typename Result, typename Service, typename... Args, typename... ArgDefinitions>
  inline constexpr auto initializer_call(Result (Service::*fn)(Args...), const ArgDefinitions & ... args) {
    return injected_calls::type_erased_injected_call_wrapper(fn, injected_calls::wire<Args...>(args...));
  }

  // J_NO_DEBUG inline constexpr const attribute_definition service_type{
  //   tag = type<struct service_type_tag>,
  //   value_type = type<type_value>};

  template<typename Type = void>
  constexpr service_reference service(const char * name = nullptr) {
    return service_reference(&type_id::get_type_id<Type>, name);
  }

  template<>
  inline constexpr service_reference service<void>(const char * name) {
    return service_reference(name);
  }
}
