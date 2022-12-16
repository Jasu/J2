#include "const_table.hpp"
#include "containers/hash_map.hpp"
#include "lisp/assembly/buffers/buffer_builder.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::lisp::assembly::buffer, u32_t, HASH(j::lisp::assembly::buffer_hash));

namespace j::lisp::compilation {
  u32_t const_table::add_const(assembly::buffer && mem) {
    auto p = map.emplace(static_cast<assembly::buffer &&>(mem), size);
    if (p.second) {
      size += p.first->first.data.size();
    }
    return p.first->second;
  }

  void const_table::add_to(assembly::buffer_builder & builder) noexcept {
    if (!size) {
      return;
    }

    u32_t base = builder.offset();
    char * ptr = builder.allocate(size);
    for (auto & p : map) {
      p.first.data.copy_to(ptr + p.second);
      for (auto & reloc : p.first.relocs) {
        builder.add_reloc(reloc, base + p.second);
      }
    }
  }
}
