#pragma once

#include "containers/vector.hpp"
#include "mem/shared_ptr.hpp"
#include "services/detail/dependencies_t.hpp"

namespace j::services::arg {
  template<typename Arg, typename InterfaceType>
  struct service_collection_adapter;

  // Vector of shared pointers
  template<typename Arg, typename InterfaceType>
  struct service_collection_adapter<mem::shared_ptr<Arg>, InterfaceType> {
    inline constexpr static bool has_dependencies_v = false;
    static vector<mem::shared_ptr<Arg>> transform(vector<mem::shared_ptr<void>> && vec, services::detail::dependencies_t *) {
      vector<mem::shared_ptr<Arg>> result(vec.size());
      for (mem::shared_ptr<void> & v : vec) {
        result.emplace_back(static_cast<mem::shared_ptr<void> &&>(v), reinterpret_cast<InterfaceType*>(v.get()));
      }
      return result;
    }
  };

  // Vector of regular pointers
  template<typename Arg, typename InterfaceType>
  struct service_collection_adapter<Arg *, InterfaceType> {
    inline constexpr static bool has_dependencies_v = true;
    static vector<Arg *> transform(vector<mem::shared_ptr<void>> && vec, services::detail::dependencies_t * deps) {
      vector<Arg *> result(vec.size());
      for (mem::shared_ptr<void> & v : vec) {
        result.push_back(reinterpret_cast<InterfaceType*>(v.get()));
        deps->emplace(static_cast<mem::shared_ptr<void> &&>(v));
      }
      return result;
    }
  };
}
