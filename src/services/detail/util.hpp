#pragma once

#include "hzd/type_traits.hpp"

namespace j::mem {
  template<typename> class shared_ptr;
  class shared_holder;
}

namespace j::inline containers {
  template<typename> class vector;
  template<typename, int> class obstack;
}

namespace j::services::detail {
  using dependencies_t J_NO_DEBUG_TYPE = obstack<mem::shared_holder, 8>;

  template<typename T>
  struct service_base_type;

  template<typename T>
  struct service_base_type<T &> {
    using type J_NO_DEBUG_TYPE = T;
  };

  template<typename T>
  struct service_base_type<mem::shared_ptr<T> &> {
    using type J_NO_DEBUG_TYPE = mem::shared_ptr<j::remove_const_t<T>>;
  };

  template<typename T>
  struct service_base_type<const mem::shared_ptr<T> &> {
    using type J_NO_DEBUG_TYPE = mem::shared_ptr<j::remove_const_t<T>>;
  };

  template<typename T>
  struct service_base_type<T*&> {
    using type J_NO_DEBUG_TYPE = j::remove_const_t<T> *;
  };

  template<typename T>
  struct service_base_type<T* const &> {
    using type J_NO_DEBUG_TYPE = j::remove_const_t<T> *;
  };

  template<typename T>
  using service_base_type_t J_NO_DEBUG_TYPE = typename service_base_type<T &>::type;

  template<typename T>
  inline constexpr bool is_shared_ptr_vector_helper_v = false;

  template<typename T>
  inline constexpr bool is_shared_ptr_vector_helper_v<vector<mem::shared_ptr<T>> &> = true;

  template<typename T>
  inline constexpr bool is_shared_ptr_vector_helper_v<const vector<mem::shared_ptr<T>> &> = true;

  template<typename T>
  inline constexpr bool is_shared_ptr_vector_v = is_shared_ptr_vector_helper_v<T &>;

  template<typename T>
  inline constexpr bool is_ptr_vector_helper_v = false;
  template<typename T>
  inline constexpr bool is_ptr_vector_helper_v<vector<T*> &> = false;
  template<typename T>
  inline constexpr bool is_ptr_vector_helper_v<const vector<T*> &> = false;

  template<typename T>
  inline constexpr bool is_ptr_vector_v = is_ptr_vector_helper_v<T &>;
}
