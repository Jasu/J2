#include "lisp/air_to_cir/air_to_cir.hpp"
#include "logging/global.hpp"
#include "lisp/air_to_cir/air_value.hpp"
#include "lisp/air_to_cir/result_use.hpp"
#include "lisp/air_to_cir/vec_helpers.hpp"
#include "lisp/air_to_cir/value_helpers.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/air/exprs/visit.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/air/functions/air_closure.hpp"
#include "lisp/symbols/symbol.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/cir/cir_function.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/passes/cir_pass.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "containers/hash_map.hpp"
#include "strings/format.hpp"

namespace j::lisp::air_to_cir {
  namespace o = j::lisp::cir::ops::defs;
  namespace c = j::lisp::cir;
  namespace a = j::lisp::air;
  namespace e = j::lisp::air::exprs;

  using expr = air::exprs::expr;
  using t = air::expr_type;
  using in = const air::exprs::expr &;
  using ins = air::exprs::const_input_span;
  using air::exprs::input;

  namespace {
    enum class branch_type : u8_t {
      root,
      alternative,
      short_circuit,
      loop,
    };

    struct J_TYPE_HIDDEN branch_context final {
      J_ALWAYS_INLINE branch_context() noexcept = default;

      /// Construct a subcontext for either a "if"-type branch with alternatives, or for a loop.
      explicit branch_context(branch_context * J_NOT_NULL parent,
                              branch_type type,
                              c::bb_builder join_bb,
                              u8_t max_branches,
                              u8_t branch_index)
        noexcept
        : type(type),
          max_branches(max_branches),
          branch_index(branch_index),
          parent(parent),
          join_bb(join_bb)
      { }

      void enter_next_branch() noexcept {
        ++branch_index;
        if (type != branch_type::short_circuit) {
          writes.clear();
        }
      }


      branch_type type = branch_type::root;
      u8_t max_branches = 1U;
      u8_t branch_index = 0U;

      /// The previous branch.
      branch_context * parent = nullptr;
      /// The earlieset basic block where the branch(es) of the current context join back together.
      ///
      /// \note Not set for the root.
      c::bb_builder join_bb;

      /// The reaching writes in the current context for variables.
      struct write {
        air_value value;
        bool is_def;
      };
      hash_map<const air::air_var *, write> writes;

      /// The phis for the variables.
      hash_map<const air::air_var *, c::op *> phis;

      void add_define(const air::air_var * J_NOT_NULL var) {
        writes.emplace(var, write{{}, true});
      }

      void add_write(const air::air_var * J_NOT_NULL var, air_value op) {
        auto p = writes.emplace(var, write{op, false});
        p.first->second.value = op;

        if (type == branch_type::root || p.first->second.is_def) {
          return;
        }
        J_ASSUME_NOT_NULL(parent);

        c::op ** phi_ = phis.maybe_at(var);
        c::op * phi = phi_ ? *phi_ : nullptr;
        // If the phi already exists, its input is updated.
        if (phi) {
          phi->set_input_value(branch_index, op.value);
          return;
        }

        // For alternatives and short-circuits, writing to a variable should create a phi node at the join after.
        if (type != branch_type::alternative && type != branch_type::short_circuit) {
          return;
        }

        auto previous = parent->get_write(var);
        c::input phi_inputs[max_branches];
        for (u8_t i = 0; i < max_branches; ++i) {
          phi_inputs[i] = i == branch_index ? op.value : previous.value;
        }
        phi = join_bb.emplace_front(o::phi, with_metadata(mdi::comment(type == branch_type::alternative ? "WR in if" : "WR in short-circuit")), span(phi_inputs, max_branches));
        phis.emplace(var, phi);
        parent->add_write(var, air_value(phi, previous.types, previous.rep));
      }

      [[nodiscard]] air_value get_write(const air::air_var * J_NOT_NULL var) noexcept {
        auto result = writes.maybe_at(var);
        // For writes in the current branch (including phi definitions), return the write directly.
        if (result) {
          return result->value;
        }
        J_ASSUME(type != branch_type::root);
        J_ASSUME_NOT_NULL(parent);
        auto previous = parent->get_write(var);
        if (type == branch_type::alternative || type == branch_type::short_circuit) {
          return previous;
        }

        // For loops, the write requiring a Phi can happen after reads, and the phi node must
        // be inserted before those reads. Since use-def chains are not tracked, the whole
        // BB would have to be scanned again.
        // Thus, in loops a phi must be generated for reads that occur before a write in the
        // loop bjkody. When leaving the loop body, if no write occurred after the read, the phi
        // can be later deleted. (i.e. if its only inputs are the previous value and itself)
        J_ASSUME(max_branches == 2);
        c::input phi_operands[max_branches];
        for (u8_t i = 0; i < max_branches; ++i) {
          phi_operands[i] = i == branch_index ? c::input{} : previous.value;
        }
        auto phi = join_bb.emplace_front(o::phi, with_metadata(mdi::comment("Loop read")), span(phi_operands, max_branches));
        phi->set_input_value(branch_index, phi);
        phis.emplace(var, phi);
        auto val = air_value(phi, previous.types, previous.rep);
        writes.emplace(var, write{val, false});
        return val;
      }

      void finalize() noexcept {
        for (auto it = phis.begin(); it != phis.end();) {
          auto * phi = it->second;
          if (type != branch_type::loop) {
            phi->num_inputs = branch_index;
          } else {
            // Erase Φ values not written in loop.
            // All reads were added to loop header, to avoid double-scanning the input
            // due to writes potentially appearing after reads in loops.
            //
            // For loops, phis always have two inputs, first of which is the pre-loop
            // value and second is the value in loop body. Phis are initialized as follows:
            //
            // ┌───┬─────────────┐
            // │ ? │ Previous op │
            // └───┴────────⊡────┘
            //              ┊
            //              ▼
            // ┌───┬────────⊡────────┬──────────────────┐
            // │ Φ │ In #0: Pre-loop │ In #1: Loop body │◀┈┐
            // └───┴─────────────────┴────────⊡─────────┘  ┊
            //                                └┈┈┈┈┈┈┈┈┈┈┈┈┘
            //
            // If nothing is added to the loop in input #1, the phi is removed. The
            // sequence for removing it is rather specific, since:
            //
            //   1. If the phi is being read, the reads must be updated to use "Previous op"
            //   2. If all uses are just replaced with Previous op, In #1 will be added to that as well.
            //   3. The previous value must be read from the phi input, since this scope has the
            //      phi node as a write for the value.
            auto & loop_input = phi->input(1);
            if (loop_input.is_op_result() && loop_input.op_result_data.use.def == phi) {
              auto & prev_input = phi->input(0);
              J_ASSERT(prev_input.is_op_result());
              auto * prev_value = prev_input.op_result_data.use.def;
              J_ASSERT(prev_value && prev_value != phi);
              prev_input.detach();
              loop_input.detach();
              join_bb.replace_result(phi, prev_value);
              join_bb.erase_no_detach(phi);
              it = phis.erase(it);
              continue;
            }
            parent->add_write(it->first, writes.at(it->first).value);
          }
          ++it;
        }
      }
    };

    struct air_op_visitor final {
      compilation::context & ctx;
      compilation::const_table & constants;
      c::ssa_builder * ssa_builder = nullptr;
      branch_context * bc = nullptr;
      c::bb_builder bb;
      functions::func_info * func_info = nullptr;
      air::air_function * air_fn = nullptr;
      air::air_closure * air_closure = nullptr;
      u32_t label_counter = 0;
      branch_context root_bc;
      functions::calling_convention calling_convention = functions::calling_convention::none;

      air_op_visitor(compilation::context & ctx,
                     compilation::const_table & constants,
                     c::ssa_builder * J_NOT_NULL ssa_builder,
                     functions::func_info * J_NOT_NULL func_info,
                     air::air_function * J_NOT_NULL air_fn,
                     functions::calling_convention cc,
                     air::air_closure * air_closure) noexcept
        : ctx(ctx),
          constants(constants),
          ssa_builder(ssa_builder),
          bc(&root_bc),
          bb(ssa_builder->emplace_back("entry")),
          func_info(func_info),
          air_fn(air_fn),
          air_closure(air_closure),
          calling_convention(cc)
      {
      }

      air_value visit_alternative_branches(const char * label_base,
                                           span<c::bb_builder> bbs,
                                           span<const expr *> exprs,
                                           const expr * J_NOT_NULL e,
                                           result_use use)
      {
        const u8_t sz = bbs.size();
        auto label = label_counter++;
        auto after_bb = ssa_builder->emplace_back(strings::format("after_{}_{}", label_base, label), sz);
        c::input results[sz];
        branch_context new_bc(bc, branch_type::alternative, after_bb, sz, 0);
        bc = &new_bc;
        const auto loc_md = copy_metadata(e->metadata());

        for (u8_t i = 0; i < exprs.size(); ++i) {
          J_ASSUME(bc == &new_bc);
          if (i) {
            bc->enter_next_branch();
          }
          bb = bbs[i];
          results[i] = visit(exprs[i], use).value;
          bb.emplace_back(o::jmp, loc_md, after_bb.bb);
        }

        c::op * result_phi = nullptr;
        if (use != result_unused) {
          result_phi = after_bb.emplace_front(o::phi, loc_md, span(results, sz));
        }
        bc->finalize();
        bc = bc->parent;
        bb = after_bb;
        return result_phi ? wrap_expr_op(e, result_phi) : air_value();
      }

      J_ALWAYS_INLINE air_value operator()(const e::nop*, result_use) const noexcept{
        return air_value_nil;
      }

      air_value operator()(const e::call * e, result_use use) {
        auto args = e->args();
        u32_t nargs = args.size();
        c::input operands[nargs];
        for (u32_t i = 0; i < nargs; ++i) {
          operands[i] = visit(args[i].expr, ptr_used).value;
        }
        air_value fn = visit(e->closure.expr, result_used);
        c::op * result = bb.emplace_back(o::call, copy_metadata(e->metadata()), fn.value, span(operands, nargs));
        if (use == result_unused) {
          return {};
        }
        return wrap_expr_op(e, result);
      }

      air_value operator()(const e::full_call * e, result_use use) {
        auto args = e->args();
        u32_t nargs = args.size();
        c::input operands[nargs];
        for (u32_t i = 0; i < nargs; ++i) {
          operands[i] = visit(args[i].expr, ptr_used).value;
        }
        c::op * nargs_op = bb.emplace_back(o::iconst, with_metadata(mdi::result("NArgs"), mdi::copy(e->metadata())), nargs);
        c::op * arg_array_op = nullptr;
        auto lmd = copy_metadata(e->metadata());
        if (nargs) {
          arg_array_op = bb.emplace_back(o::alloca, lmd, span(operands, nargs));
        }
        c::op * result = bb.emplace_back(o::full_call,
                                         with_metadata(mdi::op(0, e->fn()), mdi::copy(e->metadata())),
                                         c::full_call_reloc(e->fn()), arg_array_op, nargs_op);
        if (nargs) {
          bb.emplace_back(o::dealloca, lmd, c::untagged_const_int(nargs));
        }
        if (use == result_unused) {
          return {};
        }
        return wrap_expr_op(e, result);
      }

      void write_object_header(
        const expr * e,
        c::op * J_NOT_NULL to_address,
        lisp::mem::lowtag_type type,
        air_value info)
      {
        u32_t type_val = (u32_t)type;
        c::input tag = info.value;
        auto md = copy_metadata(e->metadata());
        if (tag.is_constant()) {
          if (info.is_tagged()) {
            tag.const_data >>= 1U;
          }
          tag.const_data |= type_val;
        } else {
          if (info.is_tagged()) {
            tag = bb.emplace_back(o::shr, md, info.value, c::untagged_const_int(1));
          }
          if (type_val) {
            tag = bb.emplace_back(o::bor, md, tag, c::untagged_const_int(type_val));
          }
        }
        bb.emplace_back(o::mem_iwr, md, c::qword_ptr(to_address), tag);
      }

      air_value copy_u64(air_value to, air_value from, air_value sz, bool init_add) {
        auto label = label_counter++;

        c::op * diff;
        c::op * end;

        if (to.types == imm_vec_ref && to.is_tagged()) {
          to = untagged_i64(bb.emplace_back(o::iadd, to.value, c::untagged_const_int(7)));
        } else if (to.types == imm_vec_ref && to.is_untagged()) {
          to = untagged_i64(bb.emplace_back(o::iadd, to.value, c::untagged_const_int(8)));
        }
        if (from.types == imm_vec_ref && from.is_tagged()) {
          from = untagged_i64(bb.emplace_back(o::iadd, from.value, c::untagged_const_int(7)));
        } else if (from.types == imm_vec_ref && from.is_untagged()) {
          from = untagged_i64(bb.emplace_back(o::iadd, from.value, c::untagged_const_int(8)));
          // init_add = true;
        }

        diff = bb.emplace_back(o::isub, from.value, to.value);

        end = bb.emplace_back(o::lea, c::scaled_qword_ptr(to.value, sz.value));
        auto head_bb = ssa_builder->emplace_back(strings::format("memcpy_head_{}", label), 2U);
        auto loop_bb = ssa_builder->emplace_back(strings::format("memcpy_loop_{}", label), 1U);
        auto after_bb = ssa_builder->emplace_back(strings::format("after_memcpy_{}", label), 1U);

        c::input phi_ops[2] = { {}, to.value};
        c::op * to_phi = head_bb.emplace_back(o::phi, with_metadata(mdi::comment("Copy64")), phi_ops);
        head_bb.emplace_back(o::jmpc, c::condition::neq, end, to_phi, loop_bb.bb, after_bb.bb);
        c::op * offset = to_phi;
        if (init_add) {
          offset = loop_bb.emplace_back(o::iadd, to_phi, c::untagged_const_int(8U));
        }
        auto res = loop_bb.emplace_back(o::mem_ird, c::indexed_qword_ptr(offset, diff));
        loop_bb.emplace_back(o::mem_iwr, c::qword_ptr(offset), res);
        if (!init_add) {
          offset = loop_bb.emplace_back(o::iadd, to_phi, c::untagged_const_int(8U));
        }
        to_phi->set_input_value(0, offset);
        loop_bb.emplace_back(o::jmp, head_bb.bb);
        bb.emplace_back(o::jmp, head_bb.bb);
        bb = after_bb;
        return untagged_i64(end);
      }

      void copy_u64_2(const air::exprs::expr * J_NOT_NULL e, air_value to, air_value from, air_value sz, bool sz_is_bytes) {
        auto label = label_counter++;
        auto lmd = copy_metadata(e->metadata());

        J_ASSERT(sz.is_untagged());
        sz = untagged_i64(bb.emplace_back(o::icopy, lmd, sz.value));
        i64_t to_offset = 0;
        if (!from.is_range() && !sz_is_bytes) {
          J_ASSERT (from.types == imm_vec_ref);
          from = untagged_i64(bb.emplace_back(o::iadd, lmd, from.value, c::untagged_const_int(from.is_tagged() ? 7 : 8)));
        } else {
          from = untagged_i64(bb.emplace_back(o::icopy, lmd, from.value));
        }
        if (!to.is_range() && !sz_is_bytes) {
          J_ASSERT (to.types == imm_vec_ref);
          to_offset = to.is_tagged() ? 7 : 8;
        }

        auto head_bb = ssa_builder->emplace_back(strings::format("memcpy_head_{}", label), 2U);
        auto loop_bb = ssa_builder->emplace_back(strings::format("memcpy_loop_{}", label), 1U);
        auto after_bb = ssa_builder->emplace_back(strings::format("after_memcpy_{}", label), 1U);

        c::input phi_ops[2] = { {}, c::untagged_const_int(0) };
        c::op * phi = head_bb.emplace_back(o::phi, lmd, phi_ops);
        head_bb.emplace_back(o::jmpc, lmd, c::condition::neq, phi, sz.value, loop_bb.bb, after_bb.bb);

        auto res = loop_bb.emplace_back(o::mem_ird, lmd, sz_is_bytes ? c::indexed_qword_ptr(from.value, phi) : c::scaled_qword_ptr(from.value, phi, mem_scale::qword));
        if (to_offset == 8U) {
          c::op * incremented = loop_bb.emplace_back(o::iadd, lmd, phi, c::untagged_const_int(sz_is_bytes ? 8U : 1U));
          phi->set_input_value(0, incremented);
          loop_bb.emplace_back(o::mem_iwr, lmd, sz_is_bytes ? c::indexed_qword_ptr(to.value, incremented) : c::scaled_qword_ptr(to.value, incremented, mem_scale::qword), res);
        }
        if (to_offset != 8U) {
          loop_bb.emplace_back(o::mem_iwr, lmd, sz_is_bytes ? c::indexed_qword_ptr(to.value, phi) : c::scaled_qword_ptr(to.value, phi, mem_scale::qword, to_offset), res);
          phi->set_input_value(0, loop_bb.emplace_back(o::iadd, lmd, phi, c::untagged_const_int(sz_is_bytes ? 8U : 1U)));
        }

        loop_bb.emplace_back(o::jmp, lmd, head_bb.bb);
        bb.emplace_back(o::jmp, lmd, head_bb.bb);
        bb = after_bb;
      }

      c::op * alloc(const expr * e, air_value sz_qwords) {
        auto lmd = copy_metadata(e->metadata());
        auto size_md = with_metadata(mdi::result("AllocSize"), mdi::copy(e->metadata()));
        c::op *sz_bytes = nullptr, *heap_op = nullptr;
        // Load the heap symbol first, since it needs a memory access. Also,
        // loading the first function argument first looks prettier in GraphViz.
        if (sz_qwords.is_const()) {
          heap_op = bb.emplace_back(o::icopy, lmd, c::var_value_untagged_reloc(air::static_ids::id_heap));
          sz_bytes = bb.emplace_back(o::iconst, size_md, sz_qwords.const_val() * (sz_qwords.is_tagged() ? 4U : 8U) + 8U);
        } else {
          sz_bytes = bb.emplace_back(o::shl, size_md, sz_qwords.value, c::untagged_const_int(sz_qwords.is_tagged() ? 2U : 3U));
          sz_bytes = bb.emplace_back(o::iadd, lmd, sz_bytes, c::untagged_const_int(8U));
          heap_op = bb.emplace_back(o::icopy, lmd, c::var_value_untagged_reloc(air::static_ids::id_heap));
        }

        c::op * ops[2]{heap_op, sz_bytes};
        return bb.emplace_back(o::abi_call, with_metadata(mdi::copy(e->metadata())), c::abi_call_reloc(air::static_ids::id_alloc), ops);
      }

      c::op * alloc_bytes(const expr * e, air_value sz_bytes) {
        c::op *heap_op = nullptr;
        auto lmd = copy_metadata(e->metadata());
        if (sz_bytes.is_const()) {
          heap_op = bb.emplace_back(o::icopy, lmd, c::var_value_untagged_reloc(air::static_ids::id_heap));
          sz_bytes = untagged_i64(bb.emplace_back(o::iconst, with_metadata(mdi::result("AllocSize"), mdi::copy(e->metadata())), sz_bytes.is_tagged() ? sz_bytes.const_val() : (sz_bytes.const_val() >> 1U)));
        } else {
          if (sz_bytes.is_tagged()) {
            sz_bytes = untagged_i64(bb.emplace_back(o::sar, with_metadata(mdi::copy(e->metadata())), sz_bytes.value, c::untagged_const_int(1)));
          }
          heap_op = bb.emplace_back(o::icopy, lmd, c::var_value_untagged_reloc(air::static_ids::id_heap));
        }

        c::op * ops[2]{heap_op, sz_bytes.value.op_result_data.use.def};
        return bb.emplace_back(o::abi_call, lmd, c::abi_call_reloc(air::static_ids::id_alloc), ops);
      }

      air_value alloc_vector(const expr * e, air_value sz, bool is_range) {
        if (sz.is_const() && sz.const_val() == 0U) {
          return is_range ? empty_vec_range : empty_vec_untagged;
        }
        c::op * ptr = alloc(e, sz);
        if (sz.is_tagged()) {
          if (sz.is_const()) {
            sz = untagged_i64(sz.const_val() >> 1);
          } else {
            sz = untagged_i64(bb.emplace_back(o::sar, copy_metadata(e->metadata()), sz.value, c::untagged_const_int(1)));
          }
        }
        write_object_header(e, ptr, lisp::mem::lowtag_type::vec, sz);
        if (is_range) {
          return air_value(
            vec_range,
            bb.emplace_back(o::iadd, copy_metadata(e->metadata()), ptr, c::untagged_const_int(8U)),
            sz.value
          );
        }
        return untagged_vec(ptr, sz.value);
      }

      air_value operator()(const e::make_vec * e, result_use) {
        auto res = alloc_vector(e, visit(e->size.expr, result_used), e->rep() == air::rep_range);
        auto lmd = copy_metadata(e->metadata());

        if (res.size.is_constant()) {
          u32_t len = res.size.const_data;
          if (len <= 16) {
            for (u32_t i = 0; i < len; ++i) {
              bb.emplace_back(cir::ops::defs::mem_iwr, lmd, range_ptr(res, untagged_i64(i)), cir::untagged_const_int(lisp_nil_v.raw));
            }
            return res;
          }
        }

        auto label = label_counter++;
        auto head_bb = ssa_builder->emplace_back(strings::format("zero_head_{}", label), 2U);
        c::input phi_ops[2] = { {}, cir::untagged_const_int(0U) };
        c::op * index_phi = head_bb.emplace_back(o::phi, lmd, phi_ops);
        auto loop_bb = ssa_builder->emplace_back(strings::format("zero_loop_{}", label), 1U);
        auto after_bb = ssa_builder->emplace_back(strings::format("after_zero_{}", label), 1U);
        head_bb.emplace_back(o::jmpc, lmd, c::condition::neq, res.size, index_phi, loop_bb.bb, after_bb.bb);
        loop_bb.emplace_back(o::mem_iwr, lmd, range_ptr(res, untagged_i64(index_phi)), cir::untagged_const_int(lisp_nil_v.raw));
        index_phi->set_input_value(0, loop_bb.emplace_back(o::iadd, lmd, index_phi, c::untagged_const_int(1U)));
        loop_bb.emplace_back(o::jmp, lmd, head_bb.bb);
        bb.emplace_back(o::jmp, lmd, head_bb.bb);
        bb = after_bb;
        return res;
      }

      air_value operator()(const e::range_set * e, result_use) {
        air_value rng = visit(e->range.expr, ptr_used), idx = visit(e->index.expr, ptr_used), res = visit(e->value.expr, ptr_used);
        bb.emplace_back(cir::ops::defs::mem_iwr, copy_metadata(e->metadata()), range_ptr(rng, idx), res.value);
        return res;
      }

      air_value operator()(const e::range_copy * e, result_use use) {
        air_value input = visit(e->range.expr, use == size_used ? use : result_used);
        if (use == size_used || use == result_unused || use == bool_used) {
          return input;
        }
        if (input.size.is_constant() && !input.size.const_data) {
          return get_empty_vec(input.rep);
        }
        if (input.is_tagged()) {
          input = untagged_vec(bb.emplace_back(o::untag, copy_metadata(e->metadata()), /*map_arg0_to_result.apply(e->metadata())*/ imm_vec_ref, input.value), input.size);
        }
        auto sz = get_range_length(bb, e, input, !input.is_range());
        // if (!input.size) {
        //   input.size = sz.value;
        // }

        air_value result = input.is_range() ? alloc_vector(e, sz, false) : untagged_vec(alloc_bytes(e, sz));
        result.size = input.size;
        copy_u64_2(e, result, input, sz, !input.is_range());
        return maybe_tag(bb, e, result, use, e->rep());
      }

      air_value operator()(const e::vec_append * e, result_use use) {
        if (use == bool_used) {
          return air_value_true;
        }
        if (use == ptr_used) {
          use = result_used;
        }

        air_value ranges[e->vecs().size()];
        air_value sizes[e->vecs().size()];
        u32_t sz = 0U;
        u32_t static_sz = 0U;
        air_value sz_op;

        auto add_size_md = copy_metadata(e->metadata()); //map_add_size.apply(e->metadata());
        for (auto & o : e->vecs()) {
          air_value vec = visit(o.expr, use);
          if (use == result_unused) {
            continue;
          }

          air_value sz_part = get_range_length(bb, e, vec);
          if (sz_part.is_const()) {
            if (!sz_part.const_val()) {
              continue;
            }
            static_sz += sz_part.const_val();
          } else if (!sz_op) {
            if (static_sz) {
              sz_op = untagged_i64(bb.emplace_back(o::iadd, add_size_md, sz_part.value, cir::untagged_const_int(static_sz)));
              static_sz = 0U;
            } else {
              sz_op = sz_part;
            }
          } else {
            sz_op = untagged_i64(bb.emplace_back(o::iadd, add_size_md, sz_op.value, sz_part.value));
          }
          sizes[sz] = sz_part;
          ranges[sz++] = vec;
        }
        if (!sz) {
          return empty_vec_untagged;
        }

        if (use == result_unused) {
          return {};
        }
        if (static_sz) {
          if (sz_op) {
            sz_op = untagged_i64(bb.emplace_back(o::iadd, add_size_md, sz_op.value, cir::untagged_const_int(static_sz)));
          } else {
            sz_op = untagged_i64(static_sz);
          }
        }

        if (use == size_used) {
          return air_value(vec_range, cir::const_nil, sz_op.value);
        }
        auto res = alloc_vector(e, sz_op, false);
        air_value result_ptr = res;
        for (u32_t i = 0; i < sz; ++i) {
          result_ptr = copy_u64(result_ptr, ranges[i], sizes[i], false);
        }
        return res;
      }

      air_value operator()(const e::vec * J_NOT_NULL e, result_use use) {
        if (use == bool_used || use == size_used || use == result_unused) {
          visit_unused_inputs(e->elems());
          return use == size_used ? air_value(vec_range, cir::const_nil, cir::untagged_const_int(e->elems().size())) : air_value_true;
        }
        air_value result = alloc_vector(e, untagged_i64(e->elems().size()), false);
        i32_t index = 0U;
        for (auto & elem : e->elems()) {
          bb.emplace_back(cir::ops::defs::mem_iwr,
                          with_metadata(mdi::copy(e->metadata()), mdi::op(0, "Vec")),
                          // .map(mdk::op(0), "Vec", index, operand_name_format::index_bracket)
                            // .map(mdk::op(1), mdk::op(index))
                            // .apply(e->metadata()),
                          range_ptr(result, untagged_i64(index)), visit(elem.expr, ptr_used).value);
          ++index;
        }
        return result;
      }

      air_value operator()(const e::vec_build * e, result_use) {
        auto elems = e->elems();
        auto ops = e->quasi_info();
        J_ASSERT(elems.size() == ops.size());
        u32_t static_size = 0U;
        for (auto & o : ops) {
          static_size += o.known_size;
        }


        air_value values[ops.size()];
        for (u32_t i = 0U, max = ops.size(); i < max; ++i) {
          if (ops[i].known_size) {
            continue;
          }
          J_ASSERT(ops[i].type == air::exprs::quasi_oper_type::append);
          expr * ex = elems[i].expr;
          if (ex->is_const()) {
            J_ASSERT(ex->get_const().is_vec_ref());
            static_size += ex->get_const().as_vec_ref()->size();
            continue;
          }
          air_value v = visit(ex, result_used);
          if (v.has_size() && v.size.is_constant()) {
            static_size += v.size.const_data;
          }
          values[i] = v;
        }

        auto lmd = copy_metadata(e->metadata());

        air_value size;

        for (u32_t i = 0U, max = ops.size(); i < max; ++i) {
          if (!values[i] || (values[i].has_size() && values[i].size.is_constant())) {
            continue;
          }
          if (!size) {
            size = get_range_length(bb, e, values[i]);
          } else {
            size = untagged_i64(bb.emplace_back(o::iadd, lmd, size.value, range_length_input(values[i])));
          }
        }

        if (!size) {
          if (!static_size) {
            return empty_vec_untagged;
          }
          size = untagged_i64(static_size);
        } else if (static_size) {
          size = untagged_i64(bb.emplace_back(o::iadd, lmd, size.value, cir::untagged_const_int(static_size)));
        }

        air_value vec = alloc_vector(e, size, false);
        air_value write_ptr = untagged_i64(vec.value);

        u8_t static_index = 8U;
        for (u32_t i = 0U, max = ops.size(); i < max; ++i) {
          if (!values[i]) {
            values[i] = visit(elems[i].expr, result_used);
          }
          auto & source = values[i];
          J_ASSERT(source);
          switch (ops[i].type) {
          case air::exprs::quasi_oper_type::none:
            J_UNREACHABLE();
          case air::exprs::quasi_oper_type::push: {
            J_ASSERT(source.is_tagged());
            c::input in;
            if (source.is_const()) {
              u64_t raw = source.const_val();
              in = ((i64_t)raw >= I32_MIN && (i64_t)raw <= I32_MAX) ? cir::input(cir::untagged_const_int(raw)) : bb.emplace_back(o::iconst, lmd, raw);
            } else {
              in = source.value.op_result_data.use.def;
            }
            bb.emplace_back(o::mem_iwr, lmd, c::qword_ptr(write_ptr.value, static_index), in);
            static_index += 8U;
            break;
          }
          case air::exprs::quasi_oper_type::append: {
            if (source.is_const()) {
              auto vec = lisp_imm{source.const_val()}.as_vec_ref()->value();
              for (auto & imm : vec) {
                u64_t raw = imm.raw;
                c::input in = ((raw < 1U << 31) || (i64_t)raw >= I32_MIN) ? cir::input(cir::untagged_const_int(raw)) : bb.emplace_back(o::iconst, lmd, raw);
                bb.emplace_back(o::mem_iwr, lmd, c::qword_ptr(write_ptr.value, static_index), in);
                static_index += 8U;
              }
              break;
            }

            if (source.size.is_constant() && source.size.const_data <= 6) {
              for (u32_t i = 0U, max = source.size.const_data; i < max; ++i) {
                c::input val = bb.emplace_back(o::mem_ird, lmd, range_ptr(source, untagged_i64(i)));
                bb.emplace_back(o::mem_iwr, lmd , c::qword_ptr(write_ptr.value, static_index), val);
                static_index += 8U;
              }
              break;
            }

            if (static_index) {
              write_ptr = untagged_i64(bb.emplace_back(o::iadd, lmd, write_ptr.value, cir::untagged_const_int(static_index)));
              static_index = 0U;
            }

            J_ASSERT(values[i]);
            air_value source_size = get_range_length(bb, e, values[i]);
            cir::op * end = bb.emplace_back(o::lea, lmd, c::scaled_qword_ptr(write_ptr.value, source_size.value));
            air_value source_ptr = get_range_begin(bb, e, values[i]);
            cir::op * diff = bb.emplace_back(o::isub, lmd, source_ptr.value, write_ptr.value);
            auto loop_bb = ssa_builder->emplace_back(strings::format("range_append_loop_{}", label_counter), 1U);
            auto loop_end_bb = ssa_builder->emplace_back(strings::format("range_append_end_{}", label_counter), 2U);
            cir::input phi_ops[2] = {write_ptr.value, {}};
            cir::op * to_phi = loop_end_bb.emplace_back(o::phi, lmd, phi_ops);
            loop_bb.emplace_back(o::mem_iwr, lmd, c::qword_ptr(to_phi), c::indexed_qword_ptr(to_phi, diff));

            to_phi->set_input_value(1, loop_bb.emplace_back(o::iadd, lmd, to_phi, cir::untagged_const_int(8U)));

            bb.emplace_back(o::jmp, lmd, loop_end_bb.bb);
            loop_bb.emplace_back(o::jmp, lmd, loop_end_bb.bb);
            auto after_loop_bb = ssa_builder->emplace_back(strings::format("after_loop_{}", label_counter), 1U);
            loop_end_bb.emplace_back(o::jmpc, lmd, cir::condition::neq, to_phi, end, loop_bb.bb, after_loop_bb.bb);
            bb = after_loop_bb;
            write_ptr = untagged_i64(to_phi);
          }
          }
        }

        return vec;
      }

      air_value operator()(const e::range_length * J_NOT_NULL e, result_use use) {
        air_value range = visit(e->range.expr, (use == result_unused || use == bool_used) ? result_unused : size_used);
        if (use == result_unused || use == bool_used) {
          return air_value_true;
        }
        return get_range_length(bb, e, range);
      }

      air_value operator()(const e::range_get * J_NOT_NULL e, result_use use) {
        use = use == result_unused ? result_unused : result_used;
        air_value range = visit(e->range.expr, use), index = visit(e->index.expr, use);
        J_ASSERT(range.types == imm_range || range.types == imm_vec_ref);
        J_ASSERT(index.only_type() == imm_i64);
        if (use == result_unused) {
          return {};
        }
        return wrap_expr_op(e, bb.emplace_back(o::mem_ird, range_ptr(range, index)));
      }

      air_value operator()(const e::as_range * J_NOT_NULL e, result_use use) {
        air_value o = visit(e->vec.expr, use);
        if (use == result_unused || use == bool_used) {
          return air_value_true;
        }
        const bool is_tagged = o.is_tagged();
        cir::input sz, ptr;

        if (use != ptr_used) {
          sz = range_length_input(o);
        }
        if (use != size_used) {
          ptr = cir::input(bb.emplace_back(o::iadd, o.value, cir::untagged_const_int(is_tagged ? 7 : 8)));
        }
        return air_value(vec_range, ptr, sz);
      }

      air_value operator()(const e::as_vec * J_NOT_NULL e, result_use use) {
        if (e->range.expr->result.types == imm_range) {
          return operator()((const e::range_copy *) e, use);
        }
        air_value o = visit(e->input(0).expr, use);
        if (use == result_unused || use == bool_used || use == size_used) {
          return o;
        }
        if (o.size.is_constant() && o.size.const_data == 0) {
          return get_empty_vec(e->rep());
        }
        return maybe_tag(bb, e, o, use, e->rep());
      }

      air_value operator()(const e::fn_arg * J_NOT_NULL e, result_use) {
        return wrap_expr_op(e, bb.emplace_back(
                              o::fn_iarg,
                              copy_metadata(e->metadata()),
                              e->index,
                              (bool)func_info->params.has_rest,
                              (bool)func_info->params.has_static_chain));
      }

      air_value operator()(const e::fn_rest_arg * J_NOT_NULL e, result_use) {
        J_ASSERT(func_info->params.has_rest || calling_convention == functions::calling_convention::full_call);
        // Construct a range.
        auto md = copy_metadata(e->metadata());
        return air_value(plain_range,
                         bb.emplace_back(o::fn_rest_ptr, md),
                         bb.emplace_back(o::fn_rest_len, md));
      }

      air_value operator()(const e::fn_sc_arg * J_NOT_NULL e, result_use) {
        J_ASSERT(func_info->params.has_static_chain);
        // Construct a range.
        auto result = bb.emplace_back(o::fn_sc_ptr, copy_metadata(e->metadata()));
        u32_t closure_offset = (air_closure->activation_record->parent ? 16U : 8U) + air_closure->act_rec_closure_index * 8U;
        return air_value(
          bb.emplace_back(o::isub, copy_metadata(e->metadata()), result, cir::untagged_const_int(closure_offset)),
          imm_act_rec, air::rep_untagged)  ;
      }

      air_value operator()(const e::ld_const * e, result_use use) {
        return air_constant(bb, e, e->const_val, e->rep(), use);
      }

      template<OneOf<e::lex_wr, e::lex_wr_bound> E>
      air_value operator()(const E * J_NOT_NULL e, result_use use) noexcept {
        air_value wr = visit(e->value.expr, (use == size_used || use == result_used) ? result_used : ptr_used);
        bc->add_write(e->var, wr);
        return wr;
      }

      template<OneOf<e::lex_rd, e::lex_rd_bound> E>
      air_value operator()(const E * J_NOT_NULL e, result_use use) {
        return use != result_unused ? bc->get_write(e->var) : air_value{};
      }

      air_value visit(const expr * e, result_use use) {
        return e::visit(*this, e, use);
      }

      template<typename CirOp>
      air_value handle_binary_expr(CirOp & op_def, const expr * J_NOT_NULL e, ins args, result_use use) {
        u32_t sz = args.size();
        use = use == result_unused ? use : result_used;
        air_value result = visit(args[0].expr, use);
        auto md = copy_metadata(e->metadata());
        for (u32_t i = 1U; i < sz; ++i) {
          // if (i > 1U) {
          //   md = md.replace(mdk::op(0)).replace(mdk::op(1), mdk::op(i));
          // }
          result = wrap_expr_op(e, bb.emplace_back(op_def, md,
                                                   result.value, visit(args[i].expr, use).value));
        }
        return result;
      }

      template<typename CirOp>
      air_value handle_binary_expr(CirOp & J_AA(NOALIAS) op_def, const expr * J_AA(NN,NOALIAS) e, expr * J_AA(NN,NOALIAS) lhs_op, expr * J_AA(NN,NOALIAS) rhs_op, result_use use) {
        use = use == result_unused ? use : result_used;
        cir::input lhs = visit(lhs_op, use).value;
        cir::input rhs = visit(rhs_op, use).value;
        if (use == result_unused) {
          return air_value_nil;
        }
        return wrap_expr_op(e, bb.emplace_back(op_def, copy_metadata(e->metadata()), lhs, rhs));
      }
      J_ALWAYS_INLINE air_value operator()(const e::tag * e, result_use use) {
        return maybe_tag(bb, e, visit(e->untagged.expr, use), use);
      }

      J_ALWAYS_INLINE air_value operator()(const e::untag * e, result_use use) {
        return maybe_untag(bb, e, visit(e->tagged.expr, use), use);
      }

      air_value operator()(const e::ineg * e, result_use use) {
        air_value i = visit(e->value.expr, use == bool_used ? result_unused : use);
        if (use == bool_used) {
          return air_value_true;
        }
        return wrap_expr_op(e, bb.emplace_back(o::ineg, copy_metadata(e->metadata()), i.value));
      }
      J_ALWAYS_INLINE air_value operator()(const e::iadd * e, result_use use) {
        return handle_binary_expr(o::iadd, e, e->lhs.expr, e->rhs.expr, use == bool_used ? result_unused : use);
      }
      J_ALWAYS_INLINE air_value operator()(const e::imul * e, result_use use) {
        return handle_binary_expr(o::smul, e, e->lhs.expr, e->rhs.expr, use == bool_used ? result_unused : use);
      }
      J_ALWAYS_INLINE air_value operator()(const e::idiv * e, result_use use) {
        return handle_binary_expr(o::sdiv, e, e->lhs.expr, e->rhs.expr, use == bool_used ? result_unused : use);
      }
      J_ALWAYS_INLINE air_value operator()(const e::isub * e, result_use use) {
        return handle_binary_expr(o::isub, e, e->lhs.expr, e->rhs.expr, use == bool_used ? result_unused : use);
      }
      J_ALWAYS_INLINE air_value operator()(const e::eq * e, result_use use) {
        return handle_binary_expr(o::eq, e, e->lhs.expr, e->rhs.expr, use == bool_used ? ptr_used : use);
      }
      J_ALWAYS_INLINE air_value operator()(const e::neq * e, result_use use) {
        return handle_binary_expr(o::neq, e, e->lhs.expr, e->rhs.expr, use == bool_used ? ptr_used : use);
      }

      air_value parse_progn(ins ins, result_use use) {
        if (ins.empty()) {
          J_ASSERT(use == result_unused);
          return air_value_none;
        }
        auto & last = ins.pop_back();
        visit_unused_inputs(ins);
        return visit(last.expr, use);
      }

      J_ALWAYS_INLINE air_value operator()(const e::prog1 * e, result_use use) {
        if (!e->num_inputs) {
          J_ASSERT(use == result_unused);
          return air_value_none;
        }
        auto first = visit(e->in_expr(0), use);
        visit_unused_inputs(e->inputs().without_prefix(1));
        return first;
      }

      J_ALWAYS_INLINE air_value operator()(const e::progn * e, result_use use) {
        return parse_progn(e->inputs(), use);
      }

      air_value operator()(const e::let * e, result_use use) {
        for (auto v : e->vars()) {
          bc->add_define(v);
        }
        return parse_progn(e->inputs(), use);
      }

      air_value operator()(const e::closure * e, result_use use) {
        if (use == result_unused || use == bool_used) {
          return air_value_true;
        }


        if (!e->closure->activation_record) {
          assembly::relocs_t relocs(1U);
          relocs[0] = assembly::reloc{0U, assembly::reloc_source_type::fn_addr, functions::calling_convention::full_call,
                                { assembly::reloc_type::abs64, 0 },
                                { e->closure->closure_symbol->name }};
          u32_t offset = constants.add_const({ j::mem::buffer(j::mem::allocate_tag, J_PTR_SZ), static_cast<assembly::relocs_t &&>(relocs) });
          return {c::constant_reloc(offset, e->result.is_tagged() ? (i32_t)tag_closure_ref : 0), e->result.type(), e->result.rep()};
        }

        auto rec_val = visit(e->act_rec.expr, result_used);
        const bool has_parent = e->closure->activation_record->parent;
        u32_t closure_offset = (has_parent ? 16U : 8U) + e->closure->act_rec_closure_index * 8U;
        if (e->result.is_tagged()) {
          closure_offset += (u32_t)tag_closure_ref;
        }
        return wrap_expr_op(e, bb.emplace_back(
                              o::iadd,
                              copy_metadata(e->metadata()),
                              rec_val.value,
                              cir::untagged_const_int(closure_offset)));
      }

      air_value operator()(const e::fn_body * e, result_use) {
        cir::cir_ssa * ssa = ssa_builder->ssa;
        ssa->fn_enter_op = bb.emplace_back(o::fn_enter, copy_metadata(e->metadata()));
        auto ins = e->inputs();
        air_value result;
        if (!ins.empty()) {
          auto & last = ins.pop_back();
          visit_unused_inputs(ins);
          result = visit(last.expr, ptr_used);
        }
        ssa->fn_ret_op = bb.emplace_back(o::iret, copy_metadata(e->metadata()), result.value);
        ssa->return_bb = bb.bb;
        return air_value_none;
      }

      template<OneOf<e::lor, e::land> E>
      air_value operator()(const E * J_NOT_NULL e, result_use use) {
        const bool is_or = e->type == t::lor;
        auto branches = e->values();
        if (!branches) {
          return is_or ? air_value_false : air_value_true;
        }
        auto lmd = copy_metadata(e->metadata());

        u8_t num_bbs = 0U;
        cir::bb_builder bbs[e->num_inputs];
        bbs[0] = bb;
        air_value results[e->num_inputs];
        c::input results_init[e->num_inputs];
        c::bb_builder after_bb;
        result_use cur_use = use == result_unused ? bool_used : use;

        branch_context new_bc;

        for (auto & b : branches) {
          const bool is_last = b.expr->parent_pos == e->num_inputs - 1U;
          air_value branch_result = results[num_bbs] = visit(b.expr, is_last ? use : cur_use);
          results_init[num_bbs] = branch_result.value;
          if (is_last) {
            break;
          }
          const truthiness t = get_truthiness(branch_result);
          if (t == (is_or ? truthiness::always_truthy : truthiness::always_falsy)) {
            break;
          }
          if (t == (is_or ? truthiness::always_falsy : truthiness::always_truthy)) {
            continue;
          }
          if (!num_bbs++) {
            after_bb = ssa_builder->emplace_back(strings::format(is_or ? "after_or_{}" : "after_and_{}", label_counter), e->num_inputs);
            new_bc = branch_context(bc, branch_type::alternative, after_bb, e->num_inputs, 0U);
            bc = &new_bc;
          }
          bc->enter_next_branch();
          bbs[num_bbs] = ssa_builder->emplace_back(strings::format(is_or ? "or_f_{}" : "and_t_{}", label_counter), 1U);
          auto temp_bb = ssa_builder->emplace_back(strings::format(is_or ? "or_t_{}" : "and_f_{}", label_counter++), 1U);
          temp_bb.emplace_back(o::jmp, lmd, after_bb.bb);
          bb.emplace_back(o::jmpt, lmd, branch_result.value, is_or ? temp_bb.bb : bbs[num_bbs].bb, is_or ? bbs[num_bbs].bb : temp_bb.bb);
          bb = bbs[num_bbs];
        }

        if (!num_bbs) {
          return results[0];
        }
        bc->finalize();
        bc = bc->parent;
        bb.emplace_back(o::jmp, lmd, after_bb.bb);
        bb = after_bb;

        return use == result_unused
          ? air_value_none
          : wrap_expr_op(e, after_bb.emplace_front(o::phi, lmd, span(results_init, num_bbs + 1U)));
      }

      template<OneOf<e::lnot, e::to_bool> E>
      air_value operator()(const E * J_NOT_NULL e, result_use use) {
        use = use == result_used ? bool_used : use;
        air_value result = visit(e->value.expr, use);
        if (use == result_unused) {
          return air_value_none;
        }
        const bool is_not = e->type == t::lnot;

        switch (get_truthiness(result)) {
        case truthiness::always_truthy:
          return is_not ? air_value_false : air_value_true;
        case truthiness::always_falsy:
          return is_not ? air_value_true : air_value_false;
        case truthiness::unknown:
          break;
        case truthiness::none:
          J_UNREACHABLE();
        }
        auto lmd = copy_metadata(e->metadata());

        auto true_bb = ssa_builder->emplace_back(strings::format("not_t_{}", label_counter), 1U);
        auto false_bb = ssa_builder->emplace_back(strings::format("not_f_{}", label_counter), 1U);
        bb.emplace_back(o::jmpt, lmd, result.value, true_bb.bb, false_bb.bb);

        bb = ssa_builder->emplace_back(strings::format("after_not_{}", label_counter++), 2U);
        true_bb.emplace_back(o::jmp, lmd, bb.bb);
        false_bb.emplace_back(o::jmp, lmd, bb.bb);
        cir::input phi_ops[]{is_not ? c::const_false : c::const_true,
            is_not ? c::const_true : c::const_false};
        return wrap_expr_op(e, bb.emplace_back(o::phi, lmd, phi_ops));
      }


      air_value operator()(const e::b_if * J_NOT_NULL e, result_use use) {
        air_value cond = visit(e->cond.expr, bool_used);
        switch (get_truthiness(cond)) {
        case truthiness::always_truthy:
          return visit(e->then_branch.expr, use);
        case truthiness::always_falsy:
          return visit(e->else_branch.expr, use);
        case truthiness::unknown:
          break;
        case truthiness::none:
          J_UNREACHABLE();
        }

        cir::bb_builder branches[]{
          ssa_builder->emplace_back(strings::format("then_{}", label_counter), 1U),
          ssa_builder->emplace_back(strings::format("else_{}", label_counter), 1U),
        };
        bb.emplace_back(o::jmpt,
                        copy_metadata(e->metadata()),
                        cond.value, branches[0].bb, branches[1].bb);
        const expr* exprs[2]{ e->then_branch.expr, e->else_branch.expr };
        return visit_alternative_branches("if", branches, span(exprs, 2), e, use);
      }

      air_value operator()(const e::sym_val_rd * J_NOT_NULL e, result_use use) {
        if (use == result_unused) {
          return air_value_nil;
        }
        auto sym = ctx.get_symbol(e->var, compilation::initializing_var);
        if (sym->is_constant() && sym->is_initialized() && non_ptr_imm_type.has(sym->value.type()) ) {
          return air_constant(bb, e, sym->value, air::rep_tagged, use);
        } else if (sym->is_constant()) {
          cir::input sz;
          if (sym->is_initialized() && sym->value_info.types == imm_vec_ref) {
            sz = c::untagged_const_int(sym->value.as_vec_ref()->size());
          }
          return {c::var_value_tagged_reloc(e->var), sym->value_info.types, air::rep_tagged, sz};
        }
        return air_value(bb.emplace_back(o::mem_ird,
                                         with_metadata(mdi::result(e->var), mdi::copy(e->metadata())),
                                         c::var_addr_reloc(e->var)),
                         e->result.types,
                         air::rep_tagged);
      }

      air_value operator()(const e::sym_val_wr * J_NOT_NULL e, result_use use) {
        auto sym = ctx.get_symbol(e->var, compilation::writable_var);
        air_value value = visit(e->value.expr, (use == size_used || use == result_used) ? result_used : ptr_used);
        bb.emplace_back(o::mem_iwr,
                        with_metadata(mdi::op(0, e->var), mdi::result(e->var), mdi::copy(e->metadata())),
                        c::var_addr_reloc(sym->name),
                        value.is_const() ? cir::input(bb.emplace_back(o::icopy, copy_metadata(e->metadata()), value.value)) : value.value);
        return value;
      }

      air_value operator()(const e::act_rec_alloc * e, result_use) {
        auto & rec = air_fn->activation_records.find(e->index);
        J_ASSUME(rec.index == e->index);
        auto parent_rec = visit(e->parent.expr, result_used);
        u32_t size = 1U + (rec.parent ? 1U : 0U) + rec.num_closures + rec.num_vars;
        cir::op * result = alloc(e, untagged_i64(size));
        write_object_header(e, result, lisp::mem::lowtag_type::act_record, untagged_i64(size));
        u32_t offset = 8U;
        if (!parent_rec.is_nil()) {
          bb.emplace_back(o::mem_iwr,
                          copy_metadata(e->metadata()),
                          c::qword_ptr(result, offset), parent_rec.value);
          offset += 8U;
        }
        auto md = copy_metadata(e->metadata());
        for (auto & closure : rec.closures()) {
          bb.emplace_back(o::mem_iwr, md,
                          c::qword_ptr(result, offset),
                          bb.emplace_back(o::icopy, copy_metadata(e->metadata()), cir::full_call_reloc(closure->closure_symbol->name)));
          offset += 8U;
        }
        return untagged_i64(result);
      }

      u32_t act_rec_offset(air::air_var * J_NOT_NULL var) const noexcept {
        auto & rec = air_fn->activation_records.find(var->activation_record_index);
        auto var_idx = rec.get_var_index(var);
        return 8U + (rec.parent ? 8U : 0U) + rec.num_closures * 8U + var_idx * 8U;
      }

      air_value operator()(const e::act_rec_rd * e, result_use) {
        return wrap_expr_op(e, bb.emplace_back(o::mem_ird,
                                               copy_metadata(e->metadata()),
                                               c::qword_ptr(visit(e->act_rec.expr, result_used).value, act_rec_offset(e->var))));
      }

      air_value operator()(const e::act_rec_wr * e, result_use) {
        auto value = visit(e->value.expr, result_used);
        bb.emplace_back(o::mem_iwr,
                        copy_metadata(e->metadata()),
                        c::qword_ptr(visit(e->act_rec.expr, result_used).value, act_rec_offset(e->var)),
                        value.value);
        return value;
      }

      air_value operator()(const e::act_rec_parent * e, result_use use) {
        if (use == result_unused) {
          return air_value_nil;
        }
        return untagged_i64(bb.emplace_back(o::mem_ird, copy_metadata(e->metadata()), c::qword_ptr(visit(e->act_rec.expr, ptr_used).value, 8U)));
      }

      template<OneOf<e::type_assert, e::dyn_type_assert> E>
      air_value operator()(const E * J_NOT_NULL e, result_use use) {
        J_ASSERT_NOT_NULL(e->types);
        air_value value = visit(e->value.expr, (use == size_used || use == result_used) ? result_used : ptr_used);
        if (e->types.is_superset_of(value.types)) {
          return value;
        }
        auto after_bb = ssa_builder->emplace_back(strings::format("type_ok_{}", label_counter), 1U);
        auto error_bb = ssa_builder->emplace_back(strings::format("type_error{}", label_counter++), 1U);
        auto md = with_metadata(mdi::copy(e->metadata()), mdi::comment("Type check"));
        bb.emplace_back(o::jmptype, md,
                        value.value,
                        e->types & value.types,
                        after_bb.bb, error_bb.bb);
        error_bb.emplace_back(o::type_error, md, e->types);
        value.constrain_to_types(e->types);
        bb = after_bb;
        return value;
      }

      air_value operator()(const e::do_until * e, result_use) {
        auto label = label_counter++;
        auto loop_bb = ssa_builder->emplace_back(strings::format("until_{}", label), 2U);
        bb.emplace_back(o::jmp,
                        copy_metadata(e->metadata()),
                        loop_bb.bb);
        bb = loop_bb;
        branch_context new_bc(bc, branch_type::loop, loop_bb, 2, 1);
        bc = &new_bc;
        air_value result = visit(e->branch.expr, bool_used);
        auto after_bb = ssa_builder->emplace_back(strings::format("after_do_until_{}", label), 1U);
        bb.emplace_back(o::jmpt,
                        copy_metadata(e->metadata()),
                        result.value,
                        after_bb.bb, loop_bb.bb);
        new_bc.finalize();
        bb = after_bb;
        bc = bc->parent;
        return result;
      }

      void visit_unused_inputs(const ins & in) {
        for (auto & i : in) {
          if (i.expr) {
            visit(i.expr, result_unused);
          }
        }
      }
    };

    void air_to_cir_cc(functions::calling_convention cc,
                       compilation::context & ctx,
                       compilation::const_table & constants,
                       functions::func_info * J_NOT_NULL func_info,
                       air::air_function * J_NOT_NULL air_fn,
                       air::air_closure * air_closure,
                       air::exprs::expr * J_NOT_NULL body,
                       cir::cir_ssa * J_NOT_NULL ssa)
    {
      cir::ssa_builder ssa_builder{ssa, ctx.temp_pool};
      air_op_visitor visitor(ctx, constants, &ssa_builder, func_info, air_fn, cc, air_closure);
      // Generate basic blocks
      visitor.visit(body, ptr_used);
      cir::run_cir_passes(ctx, &ctx.temp_pool, func_info, cc, &ssa_builder);
    }
  }

  [[nodiscard]] J_RETURNS_NONNULL cir::cir_function * air_to_cir(
    compilation::context & ctx, air::air_function * J_NOT_NULL air_fn)
  {
    auto cir_ctx = cir::cir_context.enter(ctx.env_compiler.opts->target);
    auto cfn = &ctx.temp_pool.emplace_with_padding<cir::cir_function>(
      sizeof(cir::cir_closure) * air_fn->num_closures);
    cfn->num_closures = air_fn->num_closures;

    auto cir_closure = cfn->closures().begin();
    for (auto closure = air_fn->first_closure; closure; closure = closure->next_closure, ++cir_closure) {
      ::new (cir_closure) cir::cir_closure;
      cir_closure->symbol = closure->closure_symbol;
      J_ASSUME_NOT_NULL(closure->expr);
      air_to_cir_cc(functions::calling_convention::full_call, ctx, closure->constants, closure->func_info, air_fn, closure, closure->expr, &cir_closure->ssa_full_call);
      cir_closure->constants = static_cast<compilation::const_table &&>(closure->constants);
    }

    J_ASSUME_NOT_NULL(air_fn->body);
    air_to_cir_cc(functions::calling_convention::abi_call, ctx, ctx.constants, ctx.func_info, air_fn, nullptr, air_fn->body, &cfn->ssa_abi_call);
    air_to_cir_cc(functions::calling_convention::full_call, ctx, ctx.constants, ctx.func_info, air_fn, nullptr, air_fn->body, &cfn->ssa_full_call);
    return cfn;
  }
}
