#pragma once

#include "lisp/cir/ops/op_index.hpp"
#include "lisp/cir/locs/loc_specifier.hpp"

namespace j::lisp::cir::inline ssa {
  struct ssa_builder;
}
namespace j::lisp::cir::inline ops {
  struct op;
}
namespace j::lisp::cir::inline locs {
  struct reg_map;
}

namespace j::lisp::cir::inline liveness {
  struct live_range_event;
  struct live_range_rec;
  struct live_range_tracker;
}

namespace j::lisp::cir::reg_alloc {

  struct reg_allocation_record final {
    live_range_rec * allocated_to_rec = nullptr;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return allocated_to_rec;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !allocated_to_rec;
    }

    void reset() noexcept;
    [[nodiscard]] op_index next_use() const noexcept;
    [[nodiscard]] bool is_active() const noexcept;
  };

  struct spill_point final {
    loc reg{};
    op_index next_use{};
    bool is_full_range = false;
    bool is_inactive = false;
    bool is_hint = false;
    reg_allocation_record * rec = nullptr;

    bool operator!() const noexcept;
    explicit operator bool() const noexcept;
    bool is_better_than(const spill_point & rhs) const noexcept;
  };

  struct loc_manager final {
    loc_manager(const reg_map * J_NOT_NULL map,
                live_range_tracker * J_NOT_NULL tracker,
                ssa_builder * J_NOT_NULL ssa_builder,
                bool trace_allocations) noexcept;

    loc allocate(live_range_event & event) noexcept;
    loc allocate(live_range_rec * J_NOT_NULL rec) noexcept;

    loc allocate(live_range_event & rec, loc locs[2]) noexcept;
    loc allocate(live_range_rec * J_NOT_NULL rec, loc locs[2]) noexcept;
    loc allocate(loc_specifier spec, bool is_fp, loc locs[2]) noexcept;
    loc allocate(loc_mask mask, loc_mask hint, live_range_rec * rec) noexcept;
    loc allocate(loc_mask mask, loc_mask hint, live_range_rec * rec, loc locs[2]) noexcept;

    loc allocate_aux(loc_specifier spec, bool is_fp) noexcept;

    loc unspill(live_range_event & e) noexcept;
    loc spill(loc_mask mask, loc_mask hint, live_range_rec * rec) noexcept;
    loc spill(const loc_specifier & spec, live_range_rec * rec) noexcept;
    loc get_spill_slot(live_range_rec * J_NOT_NULL rec) noexcept;

    void add_mov_before(loc to, live_range_event & from);

    void clobber(loc_mask mask) noexcept;

    void release(loc reg_or_spill) noexcept;

    loc_mask regs;
    const reg_map * map = nullptr;
    reg_allocation_record reg_recs[32];
    live_range_tracker * tracker = nullptr;
    u32_t num_spill_slots = 0U;
    ssa_builder * ssa_builder = nullptr;

    bool trace_allocations = false;
  private:
    loc try_allocate(loc_mask mask, live_range_rec * rec) noexcept;
    [[nodiscard]] spill_point find_spill_point(loc_mask mask, loc_mask hint, op_index end) noexcept;
    void spill_free(loc reg);
    loc allocate(loc loc, live_range_rec * rec) noexcept;
  };
}
