#pragma once

namespace j::services {
  struct class_metadata;

  namespace detail::container {
    struct service_instance_state;
    class service_type;
  }

  /// Context of the current injection, passed to service factories.
  ///
  /// This can be used e.g. to pass a different object to different services. For example, the
  /// default logging system passes a separate logger for each service requesting a logger.
  ///
  /// Also, when a service requests a configuration object, the configuration can be determined
  /// by the specific service being instantiated.
  struct injection_context {
    const injection_context * parent;
    const class_metadata * interface_metadata;
    const class_metadata * service_metadata;
    detail::container::service_instance_state * service_instance_state;
  };
}
