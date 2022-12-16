#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::cir::inline passes {
  namespace {
    void do_compute_reverse_postorder(
      bb * J_NOT_NULL cur,
      bb *** J_NOT_NULL ptr,
      i32_t * J_NOT_NULL num
    ) noexcept {
      if (cur->index >= 0) {
        return;
      }
      cur->index = 0;
      for (i32_t i = cur->num_exit_edges - 1; i >= 0; --i) {
        do_compute_reverse_postorder(cur->exit_edges()[i].bb, ptr, num);
      }
      J_ASSUME(cur->index == 0);
      cur->index = --(*num);
      *(--*ptr) = cur;
    }
  }

  void reverse_postorder_pass_fn(const cir_pass_context & ctx) {
    cir_ssa * ssa = ctx.ssa_builder->ssa;
    i32_t num = ssa->size;
    J_ASSUME(num > 0);
    if (ssa->reverse_postorder_size) {
      for (auto & b : ssa->in_reverse_postorder()) {
        b.index = -1;
      }
    }
    ssa->reverse_postorder_size = ssa->size;
    ssa->reverse_postorder_ptr = reinterpret_cast<bb**>(
      ctx.ssa_builder->pool.allocate(sizeof(bb*) * num));

    bb ** ptr = ssa->reverse_postorder_ptr + num;
    J_ASSUME_NOT_NULL(ssa->entry_bb);
    do_compute_reverse_postorder(ssa->entry_bb, &ptr, &num);
    J_ASSERT(ptr == ssa->reverse_postorder_ptr);
  }

  const cir_pass reverse_postorder_pass{reverse_postorder_pass_fn, "reverse-postorder", "Sort the SSA to reverse postorder"};
}
