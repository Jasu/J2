#pragma once

#include "containers/hash_map_fwd.hpp"

J_DECLARE_EXTERN_HASH_MAP(const void *, u32_t);

namespace j::tty {
  using atom_to_keysym_t = hash_map<const void *, u32_t>;
  const atom_to_keysym_t & get_termpaint_to_keysym();
}
