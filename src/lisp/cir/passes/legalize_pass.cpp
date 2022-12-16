#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/compilation/context.hpp"

#define OP_
#define OP_NO_PTR_ARGS .max_ptr_args = 0U
#define OP_IMM64 .max_imm_width = mem_width::qword
#define OP_NO_IMM .max_imm_width = mem_width::none
#define OP_MAX_PTR_ARGS(N) .max_ptr_args = N
#define OP_APPLY(...) OP_##__VA_ARGS__
#define OP(T, ...) [(u8_t)op_type::T] = {J_FEC(OP_APPLY, __VA_ARGS__)}

namespace j::lisp::cir::inline passes {
  namespace {
    struct J_TYPE_HIDDEN legalize_info final {
      u8_t max_ptr_args = 1U;
      mem_width max_imm_width = mem_width::dword;
    };

    constinit const legalize_info legalize_data[num_op_types_v]{
      OP(icopy, IMM64),
      OP(iconst, IMM64),
      OP(phi, IMM64),
      OP(tag, NO_PTR_ARGS),
      OP(untag, NO_PTR_ARGS),

      OP(call, IMM64),
      OP(full_call, IMM64),
      OP(abi_call, IMM64),

      OP(smul, NO_IMM),
      OP(sdiv, NO_IMM),
      OP(umul, NO_IMM),
      OP(udiv, NO_IMM),

      OP(iret, NO_PTR_ARGS, NO_IMM),
      OP(fret, NO_PTR_ARGS, NO_IMM),
    };

    namespace e = ops::defs;

    struct J_TYPE_HIDDEN legalize_visitor final {
      const cir_pass_context & ctx;
      bb * bb = nullptr;
      bb_builder builder{};

      void visit_op(op * o) {
        const legalize_info & info = legalize_data[(u8_t)o->type];
        u8_t num_ptr_args = 0U;
        u8_t idx = 0U;
        for (auto & in : o->inputs()) {
          bool is_ptr_arg = in.is_mem() || in.is_var_addr() || in.is_fn_addr();
          if (is_ptr_arg && ++num_ptr_args > info.max_ptr_args)  {
            op * new_input = builder.emplace_before(o, e::icopy, copy_metadata(o->metadata()), in.copy_value());
            in.set_value(idx, new_input);
          } else if (info.max_imm_width != mem_width::qword && (in.is_reloc() || in.is_constant())) {
            mem_width w = mem_width::qword;
            if (in.is_constant()) {
              i64_t cval = (i64_t)in.const_data;
              if (cval >= I8_MIN && cval <= I8_MAX) {
                w = mem_width::byte;
              } else if (cval >= I32_MIN && cval <= I32_MAX) {
                w = mem_width::dword;
              }
            }
            if (w > info.max_imm_width) {
              op * new_input = builder.emplace_before(o, e::icopy, copy_metadata(o->metadata()), in.copy_value());
              in.set_value(idx, new_input);
            }
          }
          ++idx;
        }
      }

      void visit() {
        for (auto & b : ctx.ssa_builder->ssa->in_reverse_postorder()) {
          this->bb = &b;
          this->builder = ctx.ssa_builder->get_builder(&b);
          for (auto & o : b) {
            visit_op(&o);
          }
        }
      }
    };

    void apply_legalize(const cir_pass_context & ctx) {
      legalize_visitor v{ctx};
      v.visit();
    }
  }

  const cir_pass legalize_pass{
    apply_legalize,
    "legalize",
    "Perform machine-specific legalization",
  };
}
