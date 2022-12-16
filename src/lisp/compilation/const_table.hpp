#pragma once

#include "containers/hash_map_fwd.hpp"
#include "lisp/assembly/buffers/buffer_hash.hpp"

namespace j::lisp::assembly::inline buffers {
  class buffer_builder;
}

J_DECLARE_EXTERN_HASH_MAP(j::lisp::assembly::buffer, u32_t, HASH(j::lisp::assembly::buffer_hash));

namespace j::lisp::compilation {

  struct const_table final {
    u32_t size = 0U;

    u32_t add_const(assembly::buffer && mem);
    void add_to(assembly::buffer_builder & builder) noexcept;

    hash_map<assembly::buffer, u32_t, assembly::buffer_hash> map;
  };
}
