#include "lisp/cir/reg_alloc/loc_manager.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/locs/reg_map.hpp"
#include "lisp/cir/ssa/ssa_builder.hpp"
#include "lisp/cir/liveness/live_range_tracker.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"

namespace j::lisp::cir::reg_alloc {
  namespace o = cir::ops::defs;

  bool spill_point::operator!() const noexcept {
    return !reg;
  }

  spill_point::operator bool() const noexcept {
    return (bool)reg;
  }

  bool spill_point::is_better_than(const spill_point & rhs) const noexcept {
    if (!rhs) {
      return true;
    }
    if (is_inactive != rhs.is_inactive) {
      return is_inactive;
    }
    if (rhs.is_full_range != is_full_range) {
      return is_full_range;
    }
    if (is_hint != rhs.is_hint) {
      return is_hint;
    }
    return is_full_range ? next_use < rhs.next_use : next_use > rhs.next_use;
  }

  void reg_allocation_record::reset() noexcept {
    allocated_to_rec = nullptr;
  }

  [[nodiscard]] op_index reg_allocation_record::next_use() const noexcept {
    J_ASSUME_NOT_NULL(allocated_to_rec);
    return allocated_to_rec->index();
  }

  [[nodiscard]] bool reg_allocation_record::is_active() const noexcept {
    J_ASSUME_NOT_NULL(allocated_to_rec);
    return allocated_to_rec->is_active();
  }

  loc_manager::loc_manager(const reg_map * J_NOT_NULL map,
                           live_range_tracker * J_NOT_NULL tracker,
                           struct ssa_builder * J_NOT_NULL builder,
                           bool trace_allocations) noexcept
    : regs(map->available_mask),
      map(map),
      tracker(tracker),
      ssa_builder(builder),
      trace_allocations(trace_allocations)
  {
    J_ASSERT(regs);
  }

  void loc_manager::clobber(loc_mask mask) noexcept {
    mask &= ~regs & map->available_mask;
    while(loc reg = mask.find_and_clear_first_set()) {
      J_ASSERT(reg.is_reg());
      auto & rec = reg_recs[reg.index];
      J_ASSERT(rec);
      if (rec.allocated_to_rec->current_loc.is_reg() && rec.is_active()) {
        spill_free(reg);
      }
    }
  }

  loc loc_manager::allocate(loc loc, live_range_rec * rec) noexcept {
    if (trace_allocations) {
      J_DEBUG("      {#green_bg,white,bold}  Alloc  {/} {rainbow:+1} {}", strings::format("{}", loc), format_event_target(rec), tracker->index);
    }

    J_ASSERT(loc.is_reg());
    J_ASSERT(regs.contains(loc));
    J_ASSERT(!reg_recs[loc.index]);
    regs.remove(loc);
    reg_recs[loc.index] = {rec};
    if (rec) {
      if (rec->current_loc) {
        J_ASSERT(rec->current_loc != loc);
      }
      rec->seg->loc = loc;
    }
    return loc;
  }

  loc loc_manager::try_allocate(loc_mask mask, live_range_rec * rec) noexcept {
    J_ASSUME_NOT_NULL(map);
    return mask ? allocate(map->loc_by_mask(mask), rec) : loc{};
  }

  loc loc_manager::allocate(live_range_event & event) noexcept {
    loc locs[2];
    return allocate(event.rec, locs);
  }

  loc loc_manager::allocate(live_range_rec * J_NOT_NULL rec) noexcept {
    loc locs[2];
    return allocate(rec, locs);
  }


  loc loc_manager::allocate(loc_specifier spec, bool is_fp, loc locs[2]) noexcept {
    return allocate(spec.get_mask(is_fp), spec.get_hint(is_fp), nullptr, locs);
  }

  loc loc_manager::allocate(live_range_rec * J_NOT_NULL rec, loc locs[2]) noexcept {
    loc_specifier spec = rec->seg->loc_specifier;
    return allocate(spec.get_mask(rec->is_fp), spec.get_hint(rec->is_fp), rec, locs);
  }

  loc loc_manager::allocate(loc_mask mask, loc_mask hint, live_range_rec * rec) noexcept {
    loc locs[2];
    return allocate(mask, hint, rec, locs);
  }

  loc loc_manager::allocate_aux(loc_specifier spec, bool is_fp) noexcept {
    loc locs[2];
    return allocate(spec.get_mask(is_fp), spec.get_hint(is_fp), nullptr, locs);
  }

  loc loc_manager::allocate(live_range_event & rec, loc locs[2]) noexcept {
    loc_specifier spec = rec.rec->seg->loc_specifier;
    return allocate(spec.get_mask(rec.rec->is_fp), spec.get_hint(rec.rec->is_fp), rec.rec, locs);
  }

  loc loc_manager::allocate(loc_mask mask, loc_mask hint, live_range_rec * rec, loc locs[2]) noexcept {
    if (loc_mask available_mask = regs & mask) {
      if (locs[0] && available_mask.contains(locs[0])) {
        return allocate(locs[0], rec);
      }
      if (locs[1] && available_mask.contains(locs[1])) {
        return allocate(locs[1], rec);
      }
      if (loc res = try_allocate(available_mask & hint, rec)) {
        return res;
      }
      if (loc res = try_allocate(available_mask, rec)) {
        return res;
      }
      if (locs[0]) {
        hint.add(locs[0]);
      }
      if (locs[1]) {
        hint.add(locs[1]);
      }
    }
    return spill(mask, hint, rec);
  }

  loc loc_manager::spill(const loc_specifier & spec, live_range_rec * rec) noexcept {
    return spill(spec.get_mask(rec->is_fp), spec.get_hint(rec->is_fp), rec);
  }

  loc loc_manager::unspill(live_range_event & e) noexcept {
    J_ASSERT(e.current_loc().is_spill_slot());
    loc old_reg;
    if (e.seg->loc.is_reg()) {
      old_reg = e.seg->loc;
    }
    loc new_reg = allocate(e.seg->loc_specifier, e.rec->is_fp, (loc[]){{}, {}});
    reg_recs[new_reg.index].allocated_to_rec = e.rec;
    tracker->split_at(e.rec, tracker->index, new_reg);
    auto builder = ssa_builder->get_builder(tracker->cur_bb);
    builder.emplace_before(tracker->cur_op, o::mov,
                           copy_metadata(tracker->cur_op->metadata()),
                           op_result_input{e.op(), 0U, use_type::move, get_spill_slot(e.rec), new_reg });
    e.set_current_loc(new_reg);
    return new_reg;
  }

  loc loc_manager::spill(loc_mask mask, loc_mask hint, live_range_rec * rec) noexcept {
    auto max = rec ? rec->seg.rng->upper_bound() : tracker->index;
    // J_DEBUG("       {#bright_red}Spilling{/} for {} at {} (end={}) (mask={})", format_event_target(*rec), tracker->index, max, mask);
    J_ASSERT(mask);
    spill_point point = find_spill_point(mask, hint, max);
    J_ASSERT(point);
    if (trace_allocations) {
      J_DEBUG("      {#bright_red,bold}Spilling{/} {rainbow:+1} of {} for {} at @{}", strings::format("{}", point.reg), format_event_target(point.rec->allocated_to_rec), format_event_target(rec), tracker->index);
    }
    J_ASSERT(max >= tracker->index);
    // J_DEBUG("    Spilling {} of {} next use={} ({}, {}{})", point.reg, format_event_target(*point.rec->allocated_to_rec), point.next_use,
    //         point.is_inactive ? "inactive" : "active",
    //         point.is_full_range ? "full-range" : "partial",
    //         point.is_hint ? ", with-hint" : "");

    J_ASSUME(point.rec->allocated_to_rec != rec);
    if (point.is_inactive) {
      point.rec->allocated_to_rec->current_loc = {};
      return point.reg;
    } else {
      loc spill_slot = get_spill_slot(point.rec->allocated_to_rec);
      tracker->split_at(point.rec->allocated_to_rec, tracker->index, spill_slot);
      auto builder = ssa_builder->get_builder(tracker->cur_bb);
      builder.emplace_before(tracker->cur_op, o::mov, copy_metadata(tracker->cur_op->metadata()), op_result_input{point.rec->allocated_to_rec->op, 0U, use_type::move, point.rec->allocated_to_rec->current_loc, spill_slot});
      point.rec->allocated_to_rec->current_loc = spill_slot;
    }
    point.rec->allocated_to_rec = rec;
    if (rec) {
      rec->current_loc = point.reg;
      rec->seg->loc = point.reg;
    }
    return point.reg;
  }

  loc loc_manager::get_spill_slot(live_range_rec * J_NOT_NULL rec) noexcept {
    if (!rec->spill_slot) {
      rec->spill_slot = loc::make_spill_slot(num_spill_slots++);
    }
    J_ASSERT(rec->spill_slot.is_spill_slot());
    return rec->spill_slot;
  }

  void loc_manager::release(loc reg_or_spill) noexcept {
    J_ASSUME_NOT_NULL(map);
    if (reg_or_spill.is_reg()) {
      if (trace_allocations) {
        J_DEBUG("      {#red_bg,bright_yellow} Release {/} {rainbow:+1} {}",
                strings::format("{}", reg_or_spill),
                format_event_target(reg_recs[reg_or_spill.index].allocated_to_rec));
      }
      J_ASSERT(!regs.contains(reg_or_spill));
      regs.add(reg_or_spill);
      reg_recs[reg_or_spill.index].reset();
    } else {
      J_ASSERT(reg_or_spill.is_spill_slot());
      J_ASSERT(reg_or_spill.spill_slot_index() < num_spill_slots);
    }
  }

  [[nodiscard]] spill_point loc_manager::find_spill_point(loc_mask mask, loc_mask hint, op_index end) noexcept {
    J_ASSERT(end);
    spill_point result;
    if (!mask) {
      return result;
    }
    for (u16_t i = 0; i < 32; ++i) {
      if (map->masks_by_loc_index[i] & mask) {
        J_ASSERT(map->ordered_locs[i].is_reg());

        const loc l = map->ordered_locs[i];
        J_ASSERT(l.is_reg());
        J_ASSERT(!regs.contains(l));
        auto & rec = reg_recs[l.index];
        op_index next_use = rec.next_use();
        spill_point point{
          .reg = l,
          .next_use = next_use,
          .is_full_range = next_use > end,
          .is_inactive = !rec.is_active(),
          .is_hint = hint.contains(l),
          .rec = &rec,
        };

        if (point.is_better_than(result)) {
          result = point;
        }

        mask.remove(map->masks_by_loc_index[i]);
        if (!mask) {
          break;
        }
      }
    }

    return result;
  }

  void loc_manager::spill_free(loc reg) {
    J_ASSERT(reg.is_reg());
    J_ASSERT(reg && !regs.contains(reg));
    auto & spill_rec = reg_recs[reg.index];
    if (trace_allocations) {
      J_DEBUG("    {#bright_red,bold}Spilling{/} (clobber) {} of {} at {}", strings::format(" {} ", reg), format_event_target(spill_rec.allocated_to_rec), tracker->index);
    }
    J_ASSERT(spill_rec);

    loc spill_slot = get_spill_slot(spill_rec.allocated_to_rec);
    tracker->split_at(spill_rec.allocated_to_rec, tracker->index, spill_slot);

    auto builder = ssa_builder->get_builder(tracker->cur_bb);
    builder.emplace_before(tracker->cur_op, o::mov, copy_metadata(tracker->cur_op->metadata()), op_result_input{spill_rec.allocated_to_rec->op, 0U, use_type::move, spill_rec.allocated_to_rec->current_loc, spill_slot});
    spill_rec.allocated_to_rec->current_loc = spill_slot;
    spill_rec.reset();
    regs.add(reg);
  }

  void loc_manager::add_mov_before(loc to, live_range_event & from) {
    J_ASSERT(to);
    J_ASSERT(from.current_loc());
    J_ASSERT(from.current_loc() != to);

    auto builder = ssa_builder->get_builder(tracker->cur_bb);
    builder.emplace_before(tracker->cur_op, o::mov, copy_metadata(tracker->cur_op->metadata()), op_result_input{from.op(), 0U, use_type::move, from.current_loc(), to});
    from.set_current_loc(to);
  }
}
