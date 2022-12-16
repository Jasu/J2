#include "lisp/air/air_package.hpp"
#include "lisp/values/static_objects.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/packages/pkg.hpp"
#include "logging/global.hpp"

namespace j::lisp::air {
  J_SPD_DEFINE_STATIC_PKG(static_ids, "%air", AIR)

  const static j::lisp::mem::object_header empty_vec = {j::lisp::mem::vec_tag, 0U, j::lisp::mem::object_hightag_flag::non_gc, 1U};
  const j::lisp::lisp_vec_ref g_empty_vec_ref(const_cast<lisp_vec*>(reinterpret_cast<const lisp_vec*>(&empty_vec)));

  u64_t lisp_alloc(u64_t * values, u32_t num_values) {
    // J_DEBUG("In alloc", values[0], values[1], num_values);
    J_ASSUME(num_values == 2);
    J_ASSUME_NOT_NULL(values);
    J_ASSUME(values[0] > 0);
    J_ASSUME(values[1] >= 8);
    return (u64_t)((lisp::mem::heap*)values[0])->allocate(values[1]);
  }

  u64_t lisp_alloc_abi(lisp::mem::heap * heap, u32_t size_bytes) {
    J_ASSUME_NOT_NULL(heap);
    // J_DEBUG("In alloc ABI Heap: {}, Sz:{}", (void*)heap, size_bytes);
    return (u64_t)(heap->allocate(size_bytes));
  }

  namespace {
    u64_t lisp_debug(u64_t * values, u32_t num_values) {
      J_DEBUG("{} values", num_values);
      if (num_values && !values) {
        J_ERROR("  Values array is nullptr.");
        return nil_v;
      }
      for (u32_t i = 0; i < num_values; ++i) {
        J_DEBUG("  Value {#bold}#{}:{/} {} 0x{:016X}", i, lisp_imm{values[i]}, values[i]);
      }
      return nil_v;
    }
  }

  J_RETURNS_NONNULL packages::pkg * create_air_package(env::environment * J_NOT_NULL env) {
    packages::pkg * pkg = env->package_at(package_id_air);
    pkg->register_const(static_ids::id_heap, lisp_i64((u64_t)&env->heap));
    pkg->register_const(static_ids::id_empty_vec, g_empty_vec_ref);
    pkg->register_foreign_function(static_ids::id_alloc, lisp_alloc, (void*)lisp_alloc_abi);
    pkg->register_foreign_function(static_ids::id_debug, lisp_debug);

    auto & st = pkg->symbol_table;
    J_ASSERT(st.size() == st.names.size());

    return pkg;
  }
}
