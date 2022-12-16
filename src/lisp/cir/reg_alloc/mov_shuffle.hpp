#pragma once

#include "logging/global.hpp"
#include "bits/bitset.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/ssa/bb_builder.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/cir/locs/loc_mask.hpp"

namespace j::lisp::cir::reg_alloc {
  struct mov final {
    loc to{};
    loc from_loc{};
    op * from_def = nullptr;
    input * from_input = nullptr;
  };

  using mov_set = span<mov>;

  [[nodiscard]] inline J_RETURNS_NONNULL mov * find_source(mov_set movs, loc to) noexcept {
    for (auto & mov : movs) {
      if (mov.to == to) {
        return &mov;
      }
    }
    J_ASSERT_FAIL("Source not found.");
  }

  [[nodiscard]] inline J_RETURNS_NONNULL mov * find_target(mov_set movs, loc from) noexcept {
    for (auto & mov : movs) {
      if (mov.from_loc == from) {
        return &mov;
      }
    }
    J_ASSERT_FAIL("Target not found.");
  }

  inline void mov_shuffle(bb_builder & bb, mov_set movs) {
    namespace o = cir::ops::defs;
    u32_t max_index = 0;
    for (auto & mov : movs) {
      J_ASSERT(mov.to);
      max_index = max(max_index, mov.to.index);
      if (mov.from_loc) {
        J_ASSERT(mov.from_def);
        max_index = max(max_index, mov.from_loc.index);
      }
    }
    bits::bitset to_mask(max_index + 1), from_mask(max_index + 1);
    for (auto & mov : movs) {
      to_mask.set(mov.to.index);
      if (mov.from_loc) {
        from_mask.set(mov.from_loc.index);
      }
    }

    J_ASSERT_NOT_NULL(bb.bb->last_op);
    op * src_pos = bb.bb->last_op;
    op * last_mov = src_pos->previous;
    auto src_pos_map = copy_metadata(src_pos->metadata());
    while (!to_mask.empty()) {
      while (to_mask != from_mask) {
        bits::bitset allowed_movs = to_mask - from_mask;
        u8_t to_index = allowed_movs.find_first_set();
        to_mask.reset(to_index);
        loc to_loc = loc::from_raw_u16(to_index);
        J_ASSERT(to_loc);
        const mov * m = find_source(movs, to_loc);
        if (!m->from_loc) {
          // Constant mov (or relocation or something)
          J_ASSERT_NOT_NULL(m->from_input);
          J_ASSERT(!m->from_input->is_op_result());
          last_mov = bb.emplace_after(last_mov, o::icopy,
                                      src_pos_map,
                                      m->from_input->copy_value());
          last_mov->result.set_loc(to_loc);
        } else if (to_loc.is_spill_slot() && m->from_loc.is_spill_slot()) {
          // Spill slot to spill slot move.
          auto push_type = m->from_loc.is_fp() ? mem_width::dword : mem_width::qword;
          last_mov = bb.emplace_after(last_mov, o::push,
                                      src_pos_map,
                                      push_type, m->from_loc);
          last_mov = bb.emplace_after(last_mov, o::pop,
                                      src_pos_map,
                                      push_type, to_loc);
          from_mask.reset(m->from_loc.index);
        } else {
          J_ASSERT_NOT_NULL(m->from_def);
          last_mov = bb.emplace_after(last_mov, o::mov,
                                      src_pos_map,
                                      op_result_input{m->from_def, 0U, use_type::move, m->from_loc, to_loc});
          from_mask.reset(m->from_loc.index);
        }
      }

      if (!to_mask.empty()) {
        u8_t from_index = from_mask.find_and_clear_first_set();
        loc from_loc = loc::from_raw_u16(from_index);
        loc to_loc = find_target(movs, from_loc)->to;
        J_ASSERT(from_loc && to_loc && from_loc != to_loc);

        auto reg_type = from_loc.is_fp() ? mem_width::dword : mem_width::qword;
        if (to_loc.is_reg() && from_loc.is_reg()) {
          last_mov = bb.emplace_after(last_mov, o::swap,
                                      src_pos_map,
                                      from_loc, to_loc);
        } else {
          last_mov = bb.emplace_after(last_mov, o::push, src_pos_map, reg_type, from_loc);
          last_mov = bb.emplace_after(last_mov, o::pop, src_pos_map, reg_type, to_loc);
        }
        to_mask.reset(to_loc.index);

        auto next_mov = find_target(movs, from_loc);
        J_ASSERT(next_mov->to);
        if (next_mov->to == from_loc) {
          // Simple swap
          from_mask.reset(to_loc.index);
          to_mask.reset(from_loc.index);
        }

      }
    }
    J_ASSERT(from_mask.empty());
  }
}
