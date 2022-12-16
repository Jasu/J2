#include "lisp/cir/reg_alloc/mov_shuffle.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/debug/live_range_dump.hpp"
#include "lisp/cir/liveness/live_range.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/passes/cir_pass.hpp"
#include "logging/global.hpp"

namespace j::lisp::cir::reg_alloc {
  namespace {
    struct J_TYPE_HIDDEN resolve_pass final {
      cir_ssa * ssa;
      ssa_builder * ssa_builder;

      void resolve_entry_edge(bb & block, span<live_range*> live_ins, bb_edge & e, u8_t entry_index) {
        mov movs[32];
        i32_t num_movs = 0;
        for (op & phi : block) {
          J_ASSUME(num_movs < 32);
          if (phi.type != op_type::phi) {
            break;
          }
          auto & phi_input = phi.input(entry_index);
          loc move_to = phi.result.loc_out;
          if (phi_input.is_op_result()) {
            op * from_def = phi_input.op_result_data.use.def;
            loc from_loc = from_def->live_range.segment_at(e.bb->end_index.prev_half())->loc;
            J_ASSERT(from_loc);
            if (!phi_input.op_result_data.use.loc_in) {
              phi_input.op_result_data.use.set_loc(move_to);
            }
            if (move_to == from_loc) {
              continue;
            }

            // J_DEBUG("Phi mov from op {:compact}", move_to, from_loc, *from_def);
            movs[num_movs++] = mov{move_to, from_loc, from_def, &phi_input};
          } else {
            // J_DEBUG("NonPhi mov {} = {}; Phi: ", move_to, phi_input, phi);
            movs[num_movs++] = mov{move_to, {}, nullptr, &phi_input};
          }
        }

        for (live_range * live : live_ins) {
          J_ASSERT_NOT_NULL(live);
          op * from_def = live->source_op();
          J_ASSERT_NOT_NULL(from_def);
          auto seg = live->segment_at(block.begin_index);
          J_ASSERT(seg);
          loc from_loc = live->segment_at(e.bb->end_index.prev_half())->loc;
          if (from_loc.is_spill_slot()) {
            continue;
          }
          loc move_to = seg->loc;
          J_ASSERT(move_to);
          if (from_loc == move_to) {
            continue;
          }
          J_ASSUME(num_movs < 32);
          // J_DEBUG("LiveIn mov", move_to, from_loc, *from_def, seg->index_begin, seg->index_end, block.begin_index, block.end_index, e.bb->begin_index, e.bb->end_index);
          movs[num_movs++] = mov{move_to, from_loc, from_def, nullptr};
        }

        if (num_movs) {
          bb_builder edge_builder;
          // if (e.bb->num_exit_edges == 1U) {

          if (e.bb->num_exit_edges == 1U) {
            // J_DEBUG("Using builder of previous block between {} and {}", e.bb->name, block.name);
            edge_builder = ssa_builder->get_builder(e.bb);
          } else {
            // J_DEBUG("Creating new BB block between {} and {}", e.bb->name, block.name);
            edge_builder = ssa_builder->create_between(e.bb, &block);
          }

          // } else {
          //   edge_builder = ctx.ssa_builder->emplace_back(strings::format("{}_to_{}", e.bb->name, block.name), 1U);
          //   auto jmp = edge_builder.emplace_back(o::jmp, nullptr);
          //   e.bb->last_op->exit(e.index) = {
          //     edge_builder.bb,
          //     0U,
          //   };
          //   jmp->exit(0) = {
          //     &block,
          //     entry_index,
          //   };
          //   e.bb = edge_builder.bb;
          //   e.index = 0U;
          // }
          mov_shuffle(edge_builder, mov_set{movs, num_movs});
        }
      }

      void resolve() {
        for (auto & block : ssa->in_reverse_postorder()) {
          auto live_ins = block.live_ins();
          u8_t entry_index = 0U;
          for (auto & e : block.entry_edges()) {
            // J_DEBUG("Resolving edgei #{} {} -> {}", entry_index, e.bb->name, block.name);
            resolve_entry_edge(block, live_ins, e, entry_index++);
          }
        }
      }
    };

    void resolve(const cir_pass_context & ctx) {
      resolve_pass{ctx.ssa, ctx.ssa_builder}.resolve();
      if (auto p = ctx.get_dump_file_path("live_ranges.dot")) {
        live_range_dump(p, *ctx.ssa);
      }
    }
  }

  const cir_pass resolve_phis_pass{resolve, "resolve-phis", "Resolve Phi nodes and moves"};
}
