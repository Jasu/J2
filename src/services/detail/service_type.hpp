#pragma once

#include "hzd/type_traits.hpp"

namespace j::mem {
  template<typename> class shared_ptr;
}

namespace j::services::detail {
  template<typename T>
  struct service_type;

  template<typename T>
  struct service_type<T &> {
    using type J_NO_DEBUG_TYPE = T;
  };

  template<typename Service>
  struct service_type<mem::shared_ptr<Service> &> {
    using type J_NO_DEBUG_TYPE = Service;
  };

  template<typename Service>
  struct service_type<const mem::shared_ptr<Service> &> {
    using type J_NO_DEBUG_TYPE = Service;
  };

  template<typename Service>
  struct service_type<Service * &> {
    using type J_NO_DEBUG_TYPE = Service;
  };


  template<typename T>
  using service_type_t J_NO_DEBUG_TYPE = remove_const_t<typename service_type<T &>::type>;
}
