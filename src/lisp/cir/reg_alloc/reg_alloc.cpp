#include "lisp/cir/reg_alloc/loc_manager.hpp"
#include "lisp/cir/passes/cir_passes.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/cir/debug/live_range_dump.hpp"
#include "lisp/cir/liveness/live_range_tracker.hpp"
#include "lisp/cir/ops/op.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/passes/cir_pass.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"

namespace j::lisp::cir::reg_alloc {
  namespace {
    struct J_TYPE_HIDDEN reg_allocator final {
      live_range_tracker tracker;
      ssa_builder * ssa_builder;
      loc_manager locs;
      cir_ssa * ssa;
      bool trace = false;

      explicit reg_allocator(const cir_pass_context & ctx)
        : tracker(*ctx.pool, *ctx.ssa_builder->ssa, cir_context->target.reg_info, cir_context->target.ops),
          ssa_builder(ctx.ssa_builder),
          locs(&cir_context->target.reg_map, &tracker, ssa_builder, ctx.print_debug),
          ssa(ctx.ssa),
          trace(ctx.print_debug)
      { }

      void activate_inactive_ranges() {
        while (auto event = tracker.next_inactive_event()) {
          if (trace) {
            event.dump("    Activ");
          }
          switch (event.event) {
          case lr_event::entry:
            J_ASSERT(!event.current_loc() && !event.seg->loc);
            event.set_current_loc(locs.allocate(event));
            if (event.op()) {
              event.op()->result.set_loc(event.current_loc());
            }
            break;

          case lr_event::activation:
            if (!event.current_loc()) {
              if (trace) {
                J_DEBUG("      Activation without prev");
              }
              event.set_current_loc(locs.allocate(event));
            } else if (event.seg->loc && event.current_loc() != event.seg->loc && !event.seg->loc.is_spill_slot()) {
              if (trace) {
                J_DEBUG("      Activate with {}: Cur={rainbow:+1}, Seg={rainbow:+1}", event.current_loc().is_reg() ? "different register" : "unspill", strings::format("{}", event.current_loc()), strings::format("{}", event.seg->loc));
              }
              if (event.current_loc().is_reg()) {
                locs.release(event.current_loc());
                event.set_current_loc({});
              }
              loc old_loc = event.seg->loc;
              event.set_current_loc(locs.allocate(event, (loc[]){old_loc, {}}));
              event.seg->loc = event.current_loc();
            } else if (event.seg->loc_specifier.contains(event.current_loc())) {
              if (trace) {
                J_DEBUG("      Activate reusing current loc {rainbow:+1}", strings::format("{}", event.current_loc()));
              }
              event.seg->loc = event.current_loc();
            } else {

              loc reg;
              if (event.current_loc()) {
                if (trace) {
                  J_DEBUG("      Activate alloc, must move from cur {rainbow:+1}", strings::format("{}", event.current_loc()));
                }
                auto l = event.current_loc();
                locs.release(l);
                reg = locs.allocate(event);
                locs.add_mov_before(reg, event);
              } else {
                if (trace) {
                  J_DEBUG("      Activate alloc new");
                }
                reg = locs.allocate(event);
              }
              event.set_current_loc(event.seg->loc);
            }
            break;
          default:
            J_FAIL("Invalid event");
          }
        }
      }
      void deactivate_active_ranges() {
        while (auto event = tracker.next_active_event()) {
          if (trace) {
            event.dump("    Deact");
          }
          auto cur_loc = event.current_loc();
          switch (event.event) {
          case lr_event::use:
            J_ASSERT(event.current_loc());
            if (!cur_loc.is_reg() && event.use->op()->type != op_type::phi) {
              locs.unspill(event);
            }
            J_ASSERT(event.current_loc());
            event.use->set_loc(event.current_loc());
            break;
          case lr_event::enter_segment:
            if (event.seg->loc) {
              if (trace) {
                J_DEBUG("     Entering segment with preset seg loc {rainbow:+1}", strings::format("{}", event.seg->loc));
              }
              if (!event.current_loc().is_spill_slot() && !event.seg->loc.is_spill_slot() && event.current_loc() != event.seg->loc) {
                // After a spill and split:
                //
                //                       RAX Not allocated here
                // |-- RAX -|- SPILL -|- RAX -|
                // |------ BB 1 ------|- BB2 -|
                if (event.current_loc().is_reg()) {
                  if (trace) {
                    J_DEBUG("     {#bright_yellow_bg,bold,black} Swapping registers at block boundary {/} seg={rainbow:+1} cur={rainbow:+1}", strings::format("{}", event.seg->loc),
                            strings::format("{}", event.current_loc()));
                  }
                  locs.release(event.current_loc());
                  event.set_current_loc({});
                } else if (trace) {
                  J_DEBUG("     Entering segment, allocating new reg Trying to use seg={rainbow:+1}, cur is {rainbow:+1}", strings::format("{}", event.seg->loc),
                          strings::format("{}", event.current_loc()));
                }
                loc old_loc = event.seg->loc;
                locs.allocate(event, (loc[]){old_loc, {}});
                event.set_current_loc(event.seg->loc);
              }
            } else {
              if (!cur_loc.is_reg() && event.use->op()->type != op_type::phi) {
                // Don't unspill at start of the segment, otherwise the unspill could get before
                // PHI operands.
                // locs.unspill(event);
              } else if (event.seg->loc_specifier.contains(event.current_loc())) {
                if (trace) {
                  J_DEBUG("      Reusing current location {rainbow:+1}", strings::format("{}", event.current_loc()));
                }
              } else {
                auto cur_loc = event.current_loc();
                if (trace) {
                  J_DEBUG("      {#bright_yellow_bg,black,bold} Register selection no longer valid {/} Mask: {}, Cur: {rainbow:+1}", event.seg->loc_specifier, strings::format("{}", cur_loc));
                }
                locs.release(cur_loc);
                event.set_current_loc(locs.allocate(event));
              }
              J_ASSERT(event.current_loc());
              event.seg->loc = event.current_loc();
            }
            break;
          case lr_event::exit:
            if (cur_loc.is_reg()) {
              if (event.seg->loc.is_reg() && event.seg->loc != cur_loc) {
                J_WARNING("       {#bright_red_bg,bold,yellow}Range {} moved from {} to its target {} during exit event{/}", format_event_target(&event),
                          cur_loc.is_reg(), event.seg->loc);
              }
              locs.release(cur_loc);
            }
            break;
          case lr_event::exit_segment:
          case lr_event::deactivation:
            break;
          case lr_event::activation:
          case lr_event::entry:
          case lr_event::none:
            J_FAIL("Invalid event");
          }
        }
      }

      void allocate_regs() {
        auto & ops = cir_context->target.ops;
        for (auto & bb : ssa->in_reverse_postorder()) {
          tracker.enter_bb(&bb);
          if (trace) {
            J_DEBUG("{#bright_yellow_bg} {#blue,bold}@{}{/} {#black}Enter BB {#bold}{}{/}{/} {/} ", bb.begin_index, bb.name);
          }
          for (auto & op : bb) {
            tracker.enter_op(&op);
            if (trace) {
              J_DEBUG("  {#bright_green_bg,bold} {#blue}@{}{/} {#white}Enter Op {/}{/} {#bold}{:compact}{/} ", tracker.index, op);
              tracker.validate();
            }

            // // Deactivate any ranges that are no longer required.
            // // This includes operands of the current operation - they are
            // // deactivated, so that their result may be re-used.
            // deactivate_active_ranges();

            // Activate ranges that are required, so they won't be spilled.
            activate_inactive_ranges();
            // Deactivate any ranges that are no longer required.
            // This includes operands of the current operation - they are
            // deactivated, so that their result may be re-used.
            deactivate_active_ranges();

            tracker.enter_post();

            if (trace) {
              J_DEBUG("  {#red_bg,white} {#bold}@{}{/} After Op {/} {} ", tracker.index, op.type);
              tracker.validate();
            }

            if (loc_mask clobbered = ops[op.type].clobbered_regs) {
              if (trace) {
                J_DEBUG("    {#bright_red,bold}Clobbering{/} with {}", clobbered);
              }
              locs.clobber(clobbered);
            }

            if (op.num_aux_regs) {
              for (u8_t j = 0; j < op.num_aux_regs; ++j) {
                if (trace) {
                  J_DEBUG("    {#bright_yellow,bold}Allocating AUX reg{/} {}", ops.aux_specifier(op.type, j));
                }
                op.aux_reg(j) = locs.allocate_aux(ops.aux_specifier(op.type, j), ops.is_aux_fp(op.type, j));
              }
              for (u8_t j = 0; j < op.num_aux_regs; ++j) {
                if (trace) {
                  J_DEBUG("    {#bright_yellow,bold}Releasing AUX reg {rainbow:+1}", strings::format("{}", op.aux_reg(j)));
                }
                locs.release(op.aux_reg(j));
              }
            }

            activate_inactive_ranges();

            deactivate_active_ranges();
          }
        }
        ssa->num_spill_slots = locs.num_spill_slots;
      }
    };

    void allocate_regs(const cir_pass_context & ctx) {
      try {
        reg_allocator alloc(ctx);
        alloc.allocate_regs();
      } catch (...) {
        if (auto p = ctx.get_dump_file_path("live_ranges.dot", true)) {
          J_DEBUG("Error in reg_alloc, dumping to {}", p);
          J_LOG_FLUSH();
          live_range_dump(p, *ctx.ssa);
        }
        throw;
      }
      if (auto p = ctx.get_dump_file_path("live_ranges.dot")) {
        live_range_dump(p, *ctx.ssa);
      }
    }
  }

  const cir_pass allocate_regs_pass{allocate_regs, "allocate-regs", "Allocate Registers"};
}
