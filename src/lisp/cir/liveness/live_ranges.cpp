#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/liveness/live_range.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/cir/debug/live_range_dump.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/passes/cir_pass.hpp"
#include "logging/global.hpp"
#include "mem/bump_pool.hpp"
#include "containers/vector.hpp"

namespace j::lisp::cir::inline liveness {
  namespace {
    struct J_TYPE_HIDDEN live_entry final {
      op * op;
      loc_specifier spec;
    };

    struct J_TYPE_HIDDEN live_set final {
      vector<live_entry> data;

      void add(op * J_NOT_NULL op, loc_specifier spec) {
        J_ASSERT(spec);
        for (auto & it : data) {
          J_ASSUME_NOT_NULL(it.op);
          if (it.op == op) {
            auto new_spec = it.spec & spec;
            if (!new_spec) {
              new_spec = spec;
            }
            it.spec = new_spec;
            return;
          }
        }
        data.emplace_back(live_entry{op, spec});
      }

      J_ALWAYS_INLINE void add(live_entry & entry) {
        add(entry.op, entry.spec);
      }

      void remove(op * J_NOT_NULL op) noexcept {
        for (auto it = data.begin(), end = data.end(); it != end; ++it) {
          if (it->op == op) {
            data.erase(it);
            return;
          }
        }
        // This happens with unused results.
      }

      J_RETURNS_NONNULL live_entry * find(op * J_NOT_NULL op) noexcept {
        for (auto it = data.begin(), end = data.end(); it != end; ++it) {
          if (it->op == op) {
            return it;
          }
        }
        J_FAIL("Live entry ont found.");
      }

      J_ALWAYS_INLINE auto begin() noexcept { return data.begin(); }
      J_ALWAYS_INLINE auto end() noexcept { return data.end(); }
      J_ALWAYS_INLINE auto size() const noexcept { return data.size(); }
      J_ALWAYS_INLINE auto empty() const noexcept { return data.empty(); }
    };

    struct J_TYPE_HIDDEN live_ranges_pass final {
      const op_data_table & op_data;
      cir_ssa * ssa;
      j::mem::bump_pool * pool;
      live_set * live_ins = nullptr;
      live_set * cur_live_set = nullptr;

      live_ranges_pass(const cir_pass_context & ctx)
        : op_data(cir_context->target.ops),
          ssa(ctx.ssa),
          pool(ctx.pool)
      {
      }

      void transduce(op & op) const noexcept {
        if (!op.has_result) {
          return;
        }
        // auto result_alias_flags = op_data[op.type].result.alias;
        auto static_result_spec = op_data[op.type].result_specifier(&op);
        loc_specifier & result_spec = op.live_range.front()->loc_specifier;
        result_spec &= static_result_spec;
        J_ASSERT(result_spec);

        for (u8_t i = 0U; i < op.num_inputs; ++i) {
          auto & in = op.input(i);
          if (!in.is_op_result()) {
            continue;
          }
          auto input_alias_flags = op_data[op.type].params[i].alias_flags;
          live_entry * input_rec = cur_live_set->find(in.op_result_data.use.def);
          auto & input_range = in.op_result_data.use.def->live_range;
          auto input_seg = input_range.segment_at(op.index);
          loc_specifier & input_spec = input_seg->loc_specifier;

          if (input_alias_flags.result_alias() == alias_mode::disallow) {
            auto new_input_spec = input_spec.without(result_spec);
            if (!new_input_spec) {
              // J_DEBUG("InputSpec({}).without(ResultSpec({})) == {}", input_spec, result_spec, input_spec.without(result_spec));
              // auto old_begin = input_seg->index_begin;
              // input_seg->index_begin = op.index.as_post();
              // input_range.add({
              //   .index_begin = old_begin,
              //   .index_end = op.index.as_pre(),
              //   .loc_specifier = loc_specifier::any,
              // });
            } else {
              input_spec = new_input_spec;
              J_ASSERT(input_spec);
              input_rec->spec = input_rec->spec.without(result_spec);
              result_spec = result_spec.without(input_rec->spec);
              J_ASSERT(result_spec);
            }
          } else if (input_alias_flags.result_alias() == alias_mode::prefer) {
            input_rec->spec = input_rec->spec.with_hint(result_spec);
            input_spec = input_spec.with_hint(result_spec);
            J_ASSERT(input_spec);
            result_spec = result_spec.with_hint(input_rec->spec);
            J_ASSERT(result_spec);
          }
        }
      }

      void add_op_result(const bb & bb,
                         op & op,
                         op_result_input & op_res,
                         loc_specifier input_spec) {
        J_ASSUME_NOT_NULL(cur_live_set);
        J_ASSERT_NOT_NULL(input_spec);
        op_index index = op.index;
        live_range & rng = op_res.use.def->live_range;
        auto seg = rng.segment_at(index);
        if (seg && !(seg->loc_specifier & input_spec)) {
          seg->index_begin = index.as_post();
        }
        rng.add({
            .index_begin = bb.begin_index,
            .index_end = index,
            .loc_specifier = input_spec,
          });
        cur_live_set->add(op_res.use.def, input_spec.as_hint());
      }

      void set_bb_live_ins(bb & bb) {
        J_ASSUME_NOT_NULL(cur_live_set);
        if (cur_live_set->empty()) {
          return;
        }
        pool->align(alignof(live_range*));
        bb.live_ins_begin = (live_range**)pool->allocate(cur_live_set->size() * sizeof(live_range*));
        live_range ** cur = bb.live_ins_begin;
        for (auto & live_in : *cur_live_set) {
          *cur++ = &live_in.op->live_range;
        }
        bb.num_live_ins = cur_live_set->size();
      }

      void compute_bb(bb & bb) {
        J_ASSUME_NOT_NULL(live_ins);
        J_ASSERT(bb.begin_index && bb.end_index && bb.begin_index < bb.end_index);
        cur_live_set = live_ins + bb.index;

        // Loop descendant blocks.
        for (auto & exit : bb.exit_edges()) {
          J_ASSUME_NOT_NULL(exit.bb);

          // All registers that are live at the start of a successor block must be
          // live in this block too.
          for (auto & l : live_ins[exit.bb->index]) {
            cur_live_set->add(l.op, l.spec.as_hint());
          }

          // Phi inputs corresponding to the current block must be live at the end
          // of the current block.
          for (auto & phi : *exit.bb) {
            if (phi.type != op_type::phi) {
              break;
            }
            auto & phi_operand = phi.input(exit.index);
            J_ASSUME(phi_operand.type == input_type::constant || phi_operand.type == input_type::op_result);
            if (phi_operand.is_op_result()) {
              loc_specifier result_spec = phi.live_range ? phi.live_range.front()->loc_specifier : loc_specifier::any;
              cur_live_set->add(phi_operand.op_result_data.use.def, result_spec.as_hint());
            }
          }
        }

        // Make all live variables be live from the beginning of the block, and trim
        // it after.
        for (auto & rng : *cur_live_set) {
          rng.op->live_range.add({
              .index_begin = bb.begin_index,
              .index_end = bb.end_index.prev_half(),
              .loc_specifier = rng.spec,
            });
        }

        // Iterate ops of the basic block backwards
        for (auto & op : bb.in_postorder()) {
          J_ASSERT(op.index);
          auto & cur_op_data = op_data[op.type];
          if (op.has_result) {
            loc_specifier result_spec = cur_op_data.result_specifier(&op);
            auto & range = op.live_range;
            if (!range) {
              range.add({
                  .index_begin = op.index.as_post(),
                  .index_end = op.index.as_post(),
                  .loc_specifier = result_spec,
                });
            }
            J_ASSERT(range);
            auto front = range.front();
            // Trim live range to start from its defining op.
            if (op.type == op_type::phi) {
              front->index_begin = bb.begin_index;
              continue;
            }
            loc_specifier new_spec = front->loc_specifier & result_spec;
            if (!new_spec) {
              front->index_begin = op.index.next();
              range.add({
                .index_begin = op.index.as_post(),
                .index_end = op.index.as_post(),
                .loc_specifier = result_spec,
              });
              cur_live_set->remove(&op);
            } else if (is_arg_op(op.type)) {
              front->index_begin = op_index{0};
              front->loc_specifier = new_spec;
            } else {
              front->index_begin = op.index.as_post();
              front->loc_specifier = new_spec;
              cur_live_set->remove(&op);
            }
          }

          // Ensure that the operands of the op are live.
          u8_t i = 0U;
          for (auto & in : op.inputs()) {
            auto param_spec = cur_op_data.params[i++].spec;
            if (in.is_op_result()) {
              add_op_result(bb, op, in.op_result_data, param_spec);
            } else if (in.is_mem()) {
              if (in.mem_data.base.is_op_result()) {
                add_op_result(bb, op, in.mem_data.base.op_result_data, param_spec);
              }
              if (in.mem_data.index.is_op_result()) {
                add_op_result(bb, op, in.mem_data.index.op_result_data, param_spec);
              }
            }
          }

          transduce(op);
        }

        // Remove all phi outputs from live (they might be live at this point if they
        // are defined in a loop)
        for (auto & phi : bb) {
          if (phi.type != op_type::phi) {
            break;
          }
          cur_live_set->remove(&phi);
        }

        // Make all registers live to the end of the loop, if any.
        for (auto entry : bb.entry_edges()) {
          if (entry.bb->index < bb.index) {
            continue;
          }
          for (auto & l : *cur_live_set) {
            l.op->live_range.add({
                .index_begin = bb.begin_index,
                .index_end = entry.bb->end_index.prev_half(),
                .loc_specifier = l.spec,
              });
          }
        }

        set_bb_live_ins(bb);

        cur_live_set = nullptr;
      }

      void compute() {
        J_ASSUME(ssa->size > 0);
        live_set live_ins_arr[ssa->size];
        live_ins = &live_ins_arr[0];

        // Loop through SSA BBs in reverse order
        for (auto & bb : ssa->in_postorder()) {
          compute_bb(bb);
        }

        live_ins = nullptr;
      }
    };

    void compute_live_ranges(const cir_pass_context & ctx) {
      try {
        live_ranges_pass pass(ctx);
        pass.compute();
      } catch (...) {
        if (auto p = ctx.get_dump_file_path("live_ranges.dot", true)) {
          J_ERROR("Exception in live ranges, dumping to {}", p);
          live_range_dump(p, *ctx.ssa);
        }
        throw;
      }

      if (auto p = ctx.get_dump_file_path("live_ranges.dot")) {
        live_range_dump(p, *ctx.ssa);
      }
    }
  }

  const cir_pass compute_live_ranges_pass{compute_live_ranges, "live-ranges", "Compute live ranges", env::modifies_graph_tag};
}
