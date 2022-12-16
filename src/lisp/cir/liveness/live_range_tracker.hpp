#pragma once

#include "lisp/lisp_fwd.hpp"
#include "lisp/cir/liveness/live_range.hpp"

namespace j::mem {
  class bump_pool;
}
namespace j::strings::inline styling {
  struct styled_string;
}

namespace j::lisp::assembly {
  struct reg_info;
}
namespace j::lisp::cir::inline ssa {
  struct bb;
}
namespace j::lisp::cir::inline ops {
  struct op_data_table;
  struct use;
}

namespace j::lisp::cir::inline liveness {
  enum class lr_event_state : u8_t {
    initial,
    exiting,
    inactive,
    seg_begin,
    seg_mid,
    end,
  };

  enum class lr_event : u8_t {
    none,
    deactivation,
    entry,
    exit,
    activation,
    use,
    enter_segment,
    exit_segment,
  };

  struct live_range_event;

  struct live_range_rec final {
    lr_event_state state = lr_event_state::initial;
    live_range::iterator seg;
    op * op = nullptr;
    use * use = nullptr;
    bool is_fp = false;
    loc current_loc{};
    loc spill_slot{};

    live_range_rec(live_range & rng, struct op * op, struct use * use, bool is_fp) noexcept;

    op_index index() const noexcept;
    i32_t sort_index() const noexcept;

    live_range_event advance() noexcept;

    J_INLINE_GETTER bool is_active() const noexcept {
      return state != lr_event_state::inactive && state != lr_event_state::initial;
    }

    void skip_uses() noexcept;

    void dump(const char * prefix = nullptr) const noexcept;
  };

  struct live_range_event final {
    lr_event event = lr_event::none;
    op_index index{};
    live_range::iterator seg{};
    use * use = nullptr;
    live_range_rec * rec = nullptr;

    J_INLINE_GETTER loc current_loc() const noexcept {
      return rec->current_loc;
    }

    J_ALWAYS_INLINE void set_current_loc(loc l) const noexcept {
      rec->current_loc = l;
    }

    J_INLINE_GETTER struct op * op() const noexcept {
      return rec->op;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return event != lr_event::none;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return event == lr_event::none;
    }

    void dump(const char * prefix = nullptr) const noexcept;
  };


  strings::styled_string format_event_target(const live_range_event * lr);
  strings::styled_string format_event_target(const live_range_rec * lr);

  struct live_range_ptrs final {
    J_BOILERPLATE(live_range_ptrs, CTOR_CE, COPY_DEL)

    void initialize(j::mem::bump_pool & pool, u32_t size);

    live_range_rec ** begin_ptr = nullptr;
    live_range_rec ** end_ptr = nullptr;


    J_INLINE_GETTER u32_t size() const noexcept {
      return end_ptr - begin_ptr;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return begin_ptr == end_ptr;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return begin_ptr == end_ptr;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return begin_ptr != end_ptr;
    }

    J_INLINE_GETTER_NONNULL live_range_rec ** begin() noexcept {
      return begin_ptr;
    }

    J_INLINE_GETTER_NONNULL live_range_rec ** end() noexcept {
      return end_ptr;
    }

    J_INLINE_GETTER_NONNULL const live_range_rec * const * begin() const noexcept {
      return begin_ptr;
    }

    J_INLINE_GETTER_NONNULL const live_range_rec * const * end() const noexcept {
      return end_ptr;
    }

    void sort() noexcept;
    void insert(live_range_rec * J_NOT_NULL ptr) noexcept;
    void insert_back(live_range_rec * J_NOT_NULL ptr) noexcept;

    J_RETURNS_NONNULL live_range_rec ** find(live_range_rec * J_NOT_NULL rec) noexcept;

    J_RETURNS_NONNULL live_range_rec ** erase(live_range_rec * const * const J_NOT_NULL ptr) noexcept;
    J_RETURNS_NONNULL live_range_rec ** reposition(live_range_rec ** J_NOT_NULL ptr) noexcept;

    live_range_event next_event(op_index index);

    void dump_all() const;
  };

  struct live_range_tracker final {
    live_range_ptrs inactive;
    live_range_ptrs active;

    op_index index{0};
    bb * cur_bb = nullptr;
    op * cur_op = nullptr;
    const op_data_table & op_data;

    live_range_tracker(j::mem::bump_pool & pool, cir_ssa & ssa, const assembly::reg_info * J_NOT_NULL regs, const op_data_table & op_data);

    void enter_bb(bb * J_NOT_NULL b) noexcept;
    void enter_op(op * J_NOT_NULL o) noexcept;

    void split_at(live_range_rec * J_NOT_NULL rec, op_index at, loc to);

    live_range_event next_inactive_event();
    live_range_event next_active_event();

    J_ALWAYS_INLINE void enter_post() noexcept {
      J_ASSERT(index.is_pre());
      index = index.next_half();
    }

    void validate();

    void dump_all() const;
  };
}
