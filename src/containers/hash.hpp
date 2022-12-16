#pragma once

#include "hzd/crc32.hpp"
#include "hzd/concepts.hpp"

namespace j::inline containers {
  struct int_hash {
    J_A(AI,ND,NODISC) inline u32_t operator()(u64_t v) const noexcept {
      return j::crc32(11, v);
    }
  };

  struct ptr_hash {
    J_A(AI,ND,NODISC) inline u32_t operator()(const void * v) const noexcept {
      return j::crc32(11, (u64_t)v);
    }
  };

  template<typename> struct J_AT(HIDDEN) hash_helper {
    using type = int_hash;
  };

  template<typename P> struct J_AT(HIDDEN) hash_helper<P*> {
    using type = ptr_hash;
  };

  template<typename T> using hash = typename hash_helper<T>::type;
}
