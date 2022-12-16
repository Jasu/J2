#pragma once

#include "services/detail/service_type.hpp"
#include "hzd/type_traits.hpp"

namespace j::inline containers {
  template<typename> class vector;
}
namespace j::services::detail {
  template<typename T>
  struct service_collection_service_type;

  template<typename Service>
  struct service_collection_service_type<vector<Service>> {
    using type = service_type_t<Service>;
  };

  template<typename T>
  using service_collection_service_type_t = typename service_collection_service_type<j::remove_cref_t<T>>::type;
}
