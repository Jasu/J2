#include "lisp/cir/liveness/live_range_tracker.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "strings/styling/default_styles.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ops/op_data.hpp"
#include "lisp/assembly/register_info.hpp"
#include "strings/format.hpp"
#include "algo/quicksort.hpp"
#include "logging/global.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::cir::inline liveness {
  namespace {
    namespace s = strings::styles;
    inline i32_t get_sort_key(live_range_rec * lr) noexcept {
      return lr->sort_index();
    }

    J_NO_DEBUG const strings::styled_string event_states[]{
      [(u8_t)lr_event_state::initial] = strings::make_styled_string(s::green,                   "Initial  "),
      [(u8_t)lr_event_state::inactive] = strings::make_styled_string(s::light_gray.with_bold(), "Inactive "),
      [(u8_t)lr_event_state::exiting] = strings::make_styled_string(s::bright_red,              "Exiting  "),
      [(u8_t)lr_event_state::seg_begin] = strings::make_styled_string(s::bright_green,          "Seg Begin"),
      [(u8_t)lr_event_state::seg_mid] = strings::make_styled_string(s::bright_cyan,             "SegMiddle"),
      [(u8_t)lr_event_state::end] = strings::make_styled_string(s::bright_red,                  "At End   "),
    };

    J_NO_DEBUG const strings::styled_string event_types[]{
      [(u8_t)lr_event::none]          = strings::make_styled_string(s::bright_red,               "   None"),
      [(u8_t)lr_event::entry]         = strings::make_styled_string(s::bright_green.with_bold(), "  Entry"),
      [(u8_t)lr_event::exit]          = strings::make_styled_string(s::bright_red.with_bold(),   "   Exit"),
      [(u8_t)lr_event::activation]    = strings::make_styled_string(s::bright_cyan,              " Activ."),
      [(u8_t)lr_event::deactivation]  = strings::make_styled_string(s::bright_red,               "Deactv."),
      [(u8_t)lr_event::use]           = strings::make_styled_string(s::bright_magenta,           "    Use"),
      [(u8_t)lr_event::enter_segment] = strings::make_styled_string(s::green,                    "SegEntr"),
      [(u8_t)lr_event::exit_segment]  = strings::make_styled_string(s::red,                      "SegExit"),
    };
  }

  static loc_specifier compute_loc_specifier_tail(const op_data_table & op_data, live_range_rec * J_NOT_NULL lr, op_index from) noexcept {
    use * u = lr->use;
    while (u && (!u->op()->index || u->op()->index < from)) {
      u = u->next_use;
    }
    loc_specifier ret = loc_specifier::any;
    while (u) {
      if (u->type == use_type::op_result) {
        auto op = u->op();
        ret &= op_data.arg_specifier(op->type, u->input_index);
      }
      u = u->next_use;
    }
    return ret;
  }

  strings::styled_string format_event_target(const live_range_rec * rec) {
    if (!rec) {
      return strings::format_styled("{#bright_red}None");
    }
    strings::styled_string cur_loc;
    if (rec->current_loc) {
      cur_loc = strings::format_styled("{#bright_green}Cur:{/} {rainbow:+1} ", strings::format("{}", rec->current_loc));
    }
    bool has_seg = rec->seg != rec->seg.rng->end();
    strings::styled_string seg_loc;
    strings::string seg_range = "[End]";
    if (has_seg) {
      seg_range = strings::format("{}-{}", rec->seg->index_begin, rec->seg->index_end);
      if (rec->seg->loc) {
        seg_loc = strings::format_styled("{#bright_cyan}Seg:{/} {rainbow:+1} ", strings::format("{}", rec->seg->loc));
      }
    }
    strings::styled_string use;
    if (rec->use) {
      use = strings::format_styled(" {#bright_green,bold}Use@{/}{} {}", rec->use->def->index, rec->use->def->type);
    }
    strings::styled_string target_tail = strings::format_styled("{}{}{}{}", cur_loc, seg_loc, seg_range, use);
    return rec->op
      ? strings::format_styled("{#bright_cyan}@{:02} {:<11}{/} {}",
                               rec->op->index, rec->op->type, target_tail)
      : strings::format_styled("{#gray}Callee-saved{/} {#light_gray,bold}{}{/} {}",
                             rec->seg.rng->front()->loc_specifier.get_mask(rec->is_fp), target_tail);
  }

  strings::styled_string format_event_target(const live_range_event * e) {
    if (!e) {
      return strings::format_styled("{#bright_red}None");
    }
    J_ASSERT(e->rec);
    return format_event_target(e->rec);
  }

  void live_range_ptrs::initialize(j::mem::bump_pool & pool, u32_t size) {
    J_ASSERT(begin_ptr == nullptr && end_ptr == nullptr);
    pool.align(alignof(live_range_rec*));
    begin_ptr = end_ptr = (live_range_rec**)pool.allocate(sizeof(live_range_rec*) * size);
  }

  void live_range_ptrs::sort() noexcept {
    algo::quicksort(begin_ptr, end_ptr, get_sort_key);
  }

  void live_range_ptrs::insert(live_range_rec * J_NOT_NULL ptr) noexcept {
    i32_t key = ptr->sort_index();
    auto it = begin_ptr;
    for (; it != end_ptr && (*it)->sort_index() <= key; ++it) { }
    if (it != end_ptr) {
      j::memmove(it + 1, it, (end_ptr - it) * sizeof(live_range_rec*));
    }
    *it = ptr;
    ++end_ptr;
  }

  void live_range_ptrs::insert_back(live_range_rec * J_NOT_NULL ptr) noexcept {
    i32_t key = ptr->sort_index();
    auto it = end_ptr;
    for (; it != begin_ptr && (*(it - 1))->sort_index() > key; --it) { }
    if (it != end_ptr) {
      j::memmove(it + 1, it, (end_ptr - it) * sizeof(live_range_rec*));
    }
    *it = ptr;
    ++end_ptr;
  }

  live_range_rec ** live_range_ptrs::erase(live_range_rec * const * const J_NOT_NULL ptr) noexcept {
    J_ASSUME(ptr >= begin_ptr && ptr < end_ptr);
    live_range_rec ** ptr_ = const_cast<live_range_rec **>(ptr);
    if (ptr_ != end_ptr) {
      j::memmove(ptr_, ptr_ + 1, (end_ptr - ptr_ - 1) * sizeof(live_range_rec*));
    }
    --end_ptr;
    return ptr_;
  }

  live_range_rec ** live_range_ptrs::reposition(live_range_rec ** J_NOT_NULL ptr) noexcept {
    J_ASSUME(ptr >= begin_ptr && ptr < end_ptr);
    live_range_rec * const rec = *ptr;
    i32_t key = rec->sort_index();
    if (ptr != begin_ptr && key < ptr[-1]->sort_index()) {
      for (; ptr != begin_ptr && key < ptr[-1]->sort_index(); --ptr) {
        *ptr = ptr[-1];
      }
      *ptr = rec;
    } else if (ptr != end_ptr - 1 && key > ptr[1]->sort_index()) {
      auto old_ptr = ptr;
      for (; ptr != end_ptr - 1 && key > ptr[1]->sort_index(); ++ptr) {
        *ptr = ptr[1];
      }
      *ptr = rec;
      return old_ptr;
    }
    return ptr + 1;
  }

  J_RETURNS_NONNULL live_range_rec ** live_range_ptrs::find(live_range_rec * J_NOT_NULL rec) noexcept {
    for (auto it = begin_ptr; it != end_ptr; ++it) {
      if (*it == rec) {
        return it;
      }
    }
    J_FAIL("Live range rec not found.");
  }

  live_range_rec::live_range_rec(live_range & rng, struct op * op, struct use * use, bool is_fp) noexcept
    : seg(rng.begin()),
      op(op),
      use(use),
      is_fp(is_fp)
  {
    J_ASSERT(rng);
    skip_uses();
  }

  i32_t live_range_rec::sort_index() const noexcept {
    return (index().index << 1) + (state == lr_event_state::exiting);
  }

  op_index live_range_rec::index() const noexcept {
    switch (state) {
    case lr_event_state::end:
      return {};
    case lr_event_state::initial:
    case lr_event_state::inactive:
    case lr_event_state::seg_begin:
      break;
    case lr_event_state::exiting:
      return seg->index_end;
    case lr_event_state::seg_mid: {
      const op_index seg_idx = seg->index_end;
      J_ASSERT(seg_idx);
      if (!use) {
        return seg_idx;
      }
      op_index idx = use->op()->index;
      J_ASSERT(idx && seg_idx);
      return idx < seg_idx ? idx : seg_idx;
    }
    }
    return seg->index_begin;
  }

  void live_range_rec::skip_uses() noexcept {
    while (use && (!use->op()->index || use->op()->index < use->def->index)) {
      use = use->next_use;
    }
  }

  live_range_event live_range_rec::advance() noexcept {
    live_range_event event{
      .index = index(),
      .seg = seg,
      .use = use,
      .rec = this,
    };
    switch (state) {
    case lr_event_state::initial:
      event.event = lr_event::entry;
      state = lr_event_state::seg_mid;
      break;
    case lr_event_state::exiting:
      ++seg;
      event.event = lr_event::exit;
      state = lr_event_state::end;
      break;
    case lr_event_state::seg_begin:
      event.event = lr_event::enter_segment;
      state = lr_event_state::seg_mid;
      break;
    case lr_event_state::seg_mid:
      if (use && use->op()->index == event.index) {
        event.event = lr_event::use;
        use = use->next_use;
        skip_uses();
        if (seg == seg.rng->back() && !use) {
          state = lr_event_state::exiting;
        }
      }  else {
        ++seg;
        if (seg == seg.rng->end()) {
          state = lr_event_state::end;
          event.event = lr_event::exit;
        } else if (event.seg->index_end.next_half() != seg->index_begin) {
          event.event = lr_event::deactivation;
          state = lr_event_state::inactive;
        } else {
          event.event = lr_event::exit_segment;
          state = lr_event_state::seg_begin;
        }
      }
      break;
    case lr_event_state::inactive:
      state = lr_event_state::seg_mid;
      event.event = lr_event::activation;
      break;
    case lr_event_state::end:
      J_FAIL("Tried to advance event past end.");
    }
    skip_uses();
    return event;
  }

  void live_range_rec::dump(const char * prefix) const noexcept {
    J_DEBUG("{}{}@{} {} of {}", prefix ? prefix : "", prefix ? " " : "",
            index(),
            event_states[(u8_t)state],
            format_event_target(this));
  }

  void live_range_event::dump(const char * prefix) const noexcept {
    J_DEBUG("{}{} {} of {}", prefix ? prefix : "", prefix ? " " : "",
            event_types[(u8_t)event],
            format_event_target(this));
  }

  live_range_event live_range_ptrs::next_event(op_index index) {
    J_ASSERT(index);
    if (begin_ptr == end_ptr || (*begin_ptr)->index() != index) {
      if (!(begin_ptr == end_ptr || (*begin_ptr)->index() > index)) {
        J_DEBUG("FAIL {} {}",(*begin_ptr)->index(), index);
        (*begin_ptr)->dump("FAIL");
        dump_all();
      }
      J_ASSERT(begin_ptr == end_ptr || (*begin_ptr)->index() > index);
      return {};
    }
    return (*begin_ptr)->advance();
  }

  live_range_tracker::live_range_tracker(j::mem::bump_pool & pool,
                                         cir_ssa & ssa,
                                         const assembly::reg_info * J_NOT_NULL regs,
                                         const op_data_table & op_data)
    : op_data(op_data)
  {
    const u32_t num_callee_saved = regs->callee_saved.size();
    const u32_t size = ssa.num_ops + num_callee_saved;
    inactive.initialize(pool, size);
    active.initialize(pool, size);

    pool.align(alignof(live_range_rec));
    live_range_rec * rec = (live_range_rec*)pool.allocate(sizeof(live_range_rec) * size);

    // Allocate live ranges for all callee-saved registers.
    pool.align(alignof(live_range));
    live_range * callee_saved = (live_range*)pool.allocate(sizeof(live_range) * num_callee_saved);

    pool.align(alignof(use));
    use * callee_saved_uses = (use*)pool.allocate(sizeof(use) * num_callee_saved);

    auto ret_index = ssa.fn_ret_op->index.as_post();
    for (auto & reg : regs->callee_saved) {
      loc_specifier spec{loc::from_phys_reg(reg)};
      ::new (callee_saved) live_range();
      callee_saved->add({
        .index_begin{0},
        .index_end{ret_index},
        .loc_specifier = spec,
      });
      J_ASSERT(callee_saved->size == 1U);
      ::new (callee_saved_uses) use{
        .type = use_type::callee_saved,
        .def = ssa.fn_ret_op,
      };
      ::new (rec) live_range_rec(*callee_saved, nullptr, callee_saved_uses, reg.is_fp());
      inactive.insert_back(rec);
      rec++;
      ++callee_saved;
      ++callee_saved_uses;
    }

    // Allocate a record for each instruction that has a live range.
    for (auto & op : ssa.ops_in_reverse_postorder()) {
      if (op.live_range) {
        ::new (rec) live_range_rec(op.live_range, &op, op.result.next_use, op_data[op.type].result.is_fp());
        inactive.insert_back(rec);
        rec++;
      }
    }
  }

  void live_range_tracker::enter_bb(bb * J_NOT_NULL b) noexcept {
    J_ASSERT(!cur_bb || cur_bb->end_index == b->begin_index);
    cur_bb = b;
  }

  void live_range_tracker::enter_op(op * J_NOT_NULL o) noexcept {
    J_ASSUME_NOT_NULL(cur_bb);
    J_ASSERT(!o->index || (o->index >= cur_bb->begin_index && o->index < cur_bb->end_index));
    J_ASSERT(!cur_op || !o->index || o->index == index.next());
    cur_op = o;
    if (o->index) {
      index = o->index;
    }
  }

  static bool validate_rec(const char * type, live_range_rec * J_NOT_NULL rec, op_index cur_index) {
    bool had_error = false;
    op_index idx = rec->index();
    if (!idx && rec->state != lr_event_state::end) {
      J_ERROR("{} record {} had no index.", type, event_states[(u8_t)rec->state]);
      had_error = true;
    }
    if (idx && idx < cur_index) {
      J_ERROR("{} record @{} {} was out of order, (expected index >= {}",
              type, idx, event_states[(u8_t)rec->state], format_event_target(rec), cur_index);
      had_error = true;
    }
    if (idx && (rec->seg->index_end < idx || rec->seg->index_begin > idx)) {
      J_ERROR("{} record @{} {} of {} had index out of region range {}-{}",
              type, idx, event_states[(u8_t)rec->state], format_event_target(rec), rec->seg->index_begin, rec->seg->index_end);
      had_error = true;
    }
    return had_error;
  }

  void live_range_tracker::validate() {
    op_index cur_index = index;
    bool had_error = false;
    for (auto * rec : active) {
      const op_index idx = rec->index();
      had_error |= validate_rec("Active", rec, cur_index);
      switch (rec->state) {
      case lr_event_state::seg_mid:
      case lr_event_state::seg_begin: {
        if (rec->use) {
          const op_index use_index = rec->use->op()->index;
          if (!use_index || use_index < cur_index) {
            rec->dump("Active range had use with an invalid index def.");
            had_error = true;
          } else if (use_index < rec->use->def->index) {
            rec->dump("Active range had use before def.");
            had_error = true;
          }
        }
        break;
      }
      case lr_event_state::exiting:
        if (rec->use) {
          rec->dump("Active range had use but was exiting.");
          had_error = true;
        }
        break;
      default:
        rec->dump("Active range had an invalid event state.");
        had_error = true;
        break;
      }
      cur_index = idx;
    }

    cur_index = index;
    for (auto * rec : inactive) {
      had_error |= validate_rec("Inactive", rec, cur_index);
      op_index idx = rec->index();
      if (rec->is_active()) {
        rec->dump("Inactive range had invalid state.");
        had_error = true;
      }
      if (rec->seg == rec->seg.rng->end()) {
        rec->dump("Inactive range had no valid segments.");
        had_error = true;
      }
      if (rec->seg->index_end < index) {
        rec->dump("Inactive range had segment in the past.");
        had_error = true;
      }
      for (auto * rec2 : active) {
        if (rec2 == rec) {
          rec->dump("Duplicate record");
          had_error = true;
        }
      }
      cur_index = idx;
    }
    if (had_error) {
      dump_all();
      J_THROW("Live range tracker did not validate.");
    }
  }

  void live_range_tracker::split_at(live_range_rec * J_NOT_NULL lr, op_index at, loc to) {
    J_ASSERT(to && at && index && at >= index && at <= cur_bb->end_index);
    // J_DEBUG("  Splitting {}-{} of {} at {}", lr->seg->index_begin, lr->seg->index_end, format_event_target(lr), at);
    auto seg_index = lr->seg - lr->seg.rng->begin();
    auto old_loc = lr->seg->loc;
    auto seg = lr->seg.rng->move_to_loc_at(at, to);
    seg->loc_specifier = compute_loc_specifier_tail(op_data, lr, seg->index_begin);
    if (cur_bb->end_index < seg.rng->upper_bound() && seg->index_end != cur_bb->end_index.prev_half()) {
      [[maybe_unused]] auto seg2 = lr->seg.rng->move_to_loc_at(cur_bb->end_index, old_loc);
      // J_DEBUG("  Additional segment {}-{}", seg2->index_begin, seg2->index_end);
    }
    lr->seg = lr->seg.rng->begin() + seg_index;
    if (lr->seg->index_end <= index && lr->seg->index_begin != index) {
      lr->seg++;
    }

    if (lr->is_active()) {
      if (at == index) {
        lr->state = lr_event_state::seg_begin;
      }
      active.reposition(active.find(lr));
    } else {
      inactive.reposition(inactive.find(lr));
    }
  }

  live_range_event live_range_tracker::next_inactive_event() {
    live_range_event e = inactive.next_event(index);
    if (e) {
      J_ASSERT(e.index == index);
      inactive.erase(inactive.begin());
      active.insert(e.rec);
    }
    return e;
  }

  live_range_event live_range_tracker::next_active_event() {
    live_range_event e = active.next_event(index);
    switch (e.event) {
    case lr_event::entry:
    case lr_event::activation:
      J_FAIL("Invalid event type");
    case lr_event::none:
      break;
    case lr_event::use:
    case lr_event::exit_segment:
    case lr_event::enter_segment:
      J_ASSERT(e.index == index);
      active.reposition(active.begin());
      break;
    case lr_event::deactivation:
      inactive.insert(e.rec);
      [[fallthrough]];
    case lr_event::exit:
      active.erase(active.begin());
      J_ASSERT(e.index == index);
      break;
    }
    return e;
  }

  void live_range_ptrs::dump_all() const {
    for (auto * rec : *this) {
      rec->dump();
    }
  }

  void live_range_tracker::dump_all() const {
    J_DEBUG("{#bright_green_bg,white,bold} Active: ");
    active.dump_all();
    J_DEBUG("{#red_bg,white,bold} Inactive: ");
    inactive.dump_all();
  }
}
