#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/concepts.hpp"
#include "containers/detail/resizeable_region.hpp"

namespace j::inline containers {
  template<typename T>
  class noncopyable_vector : public detail::resizeable_region<T> {
  public:
    J_A(AI,ND,NE,HIDDEN) inline noncopyable_vector() noexcept = default;

    using detail::resizeable_region<T>::resizeable_region;

    J_A(AI,ND,NE,HIDDEN) inline void swap(noncopyable_vector & rhs) noexcept {
      detail::resizeable_region<T>::swap(rhs);
    }
  };

  template<typename T, u32_t I>
  explicit noncopyable_vector(const T(&)[I]) -> noncopyable_vector<T>;

  template<typename T, u32_t I>
  explicit noncopyable_vector(T(&)[I]) -> noncopyable_vector<T>;

  template<typename T>
  class vector : public detail::resizeable_region_copyable<T> {
  public:
    J_A(AI,ND,NE,HIDDEN) inline vector() noexcept = default;

    using detail::resizeable_region_copyable<T>::resizeable_region_copyable;

    J_A(AI,ND,NE,HIDDEN) inline void swap(vector & rhs) noexcept {
      detail::resizeable_region_copyable<T>::swap(rhs);
    }
  };

  template<typename T, u32_t I>
  explicit vector(const T(&)[I]) -> vector<T>;

  template<typename T, u32_t I>
  explicit vector(T(&)[I]) -> vector<T>;

  template<typename T>
  J_A(NODISC,NE,NI,HIDDEN) bool operator==(const noncopyable_vector<T> & lhs,
                  const noncopyable_vector<T> & rhs) noexcept
  {
    const u32_t sz = lhs.size();
    if (sz != rhs.size()) {
      return false;
    }
    for (u32_t i = 0U; i < sz; ++i) {
      if (lhs[i] != rhs[i]) {
        return false;
      }
    }
    return true;
  }

  template<typename T>
  J_A(NODISC,NE,NI,HIDDEN) inline bool operator==(const vector<T> & lhs, const vector<T> & rhs) noexcept {
    const u32_t sz = lhs.size();
    if (sz != rhs.size()) {
      return false;
    }
    for (u32_t i = 0U; i < sz; ++i) {
      if (lhs[i] != rhs[i]) {
        return false;
      }
    }
    return true;
  }
}

#define J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(...) extern template class j::noncopyable_vector<__VA_ARGS__>
#define J_DECLARE_EXTERN_VECTOR(...) extern template class j::vector<__VA_ARGS__>
#define J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(...) template class j::noncopyable_vector<__VA_ARGS__>
#define J_DEFINE_EXTERN_VECTOR(...) template class j::vector<__VA_ARGS__>

J_DECLARE_EXTERN_VECTOR(u32_t);
