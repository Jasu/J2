#include "lisp/cir/ssa/ssa_builder.hpp"
#include "strings/string.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/ssa/bb.hpp"

namespace j::lisp::cir::inline ssa {
  [[nodiscard]] bb_builder ssa_builder::get_builder_at(op_index at) noexcept {
    J_ASSERT(at);
    for (auto & bb : ssa->in_reverse_postorder()) {
      if (bb.end_index > at) {
        return {&bb, &pool};
      }
    }
    J_FAIL("Builder of op not found.");
  }

  [[nodiscard]] bb_builder ssa_builder::get_builder_of(op * J_NOT_NULL o) noexcept {
    return get_builder_at(o->index);
  }

  [[nodiscard]] bb_builder ssa_builder::emplace_back(strings::const_string_view name, u8_t num_entries) {
    J_ASSUME_NOT_NULL(ssa);
    bb * result = &pool.emplace_with_padding<bb>(sizeof(bb_edge) * num_entries + name.size());
    char * str = add_bytes<char *>(result + 1, sizeof(bb_edge) * num_entries);
    ::j::memcpy(str, name.begin(), name.size());
    result->name = strings::const_string_view(str, name.size());
    if (!num_entries) {
      J_ASSUME(ssa->entry_bb == nullptr && ssa->size == 0);
      ssa->entry_bb = result;
    } else {
      J_ASSUME_NOT_NULL(ssa->entry_bb, ssa->size);
      result->entry_edges_begin = reinterpret_cast<bb_edge *>(result + 1);
    }
    ++ssa->size;
    return bb_builder{result, &pool};
  }

  [[nodiscard]] bb_builder ssa_builder::create_between(bb * J_NOT_NULL from, bb * J_NOT_NULL to) noexcept {
    J_ASSUME_NOT_NULL(ssa);
    strings::string name = from->name + "_to_" + to->name;
    u32_t name_sz = name.size();

    bb * result = &pool.emplace_with_padding<bb>(sizeof(bb_edge) + name_sz);
    result->entry_edges_begin = reinterpret_cast<bb_edge *>(result + 1);
    char * str = add_bytes<char *>(result + 1, sizeof(bb_edge));
    ::j::memcpy(str, name.begin(), name_sz);
    result->name = strings::const_string_view(str, name_sz);

    u8_t entry_index = 255U;
    u8_t exit_index = 0U;
    for (auto & edge : from->exit_edges()) {
      if (edge.bb == to) {
        entry_index = edge.index;
        edge = { result, 0 };
        break;
      }
      ++exit_index;
    }
    J_ASSERT(entry_index != 255U);
    *result->entry_edges_begin = { from, exit_index };

    bb_builder b{result, &pool};

    op * o = b.emplace_front(ops::defs::jmp, with_metadata(mdi::comment(name)), nullptr);
    J_ASSERT(o->num_exits == 1);
    result->num_exit_edges = result->num_entry_edges = o->num_exits = 1U;
    result->exit_edges_begin = o->exits_begin();
    ::new (result->exit_edges_begin) bb_edge{to, entry_index};
    J_ASSERT(to->entry_edges()[entry_index].bb == from);
    J_ASSERT(to->entry_edges()[entry_index].index == exit_index);
    to->entry_edges()[entry_index] = { result, 0U };

    ++ssa->size;
    return b;
  }


  void ssa_builder::replace_bb_with(bb * J_NOT_NULL old_bb, bb * J_NOT_NULL new_bb) {
    J_ASSUME_NOT_NULL(ssa);
    J_ASSUME(old_bb != new_bb);
    u32_t num_edges = old_bb->num_entry_edges + new_bb->num_entry_edges;
    bb_edge * edges = (bb_edge*)pool.allocate(num_edges * sizeof(bb_edge));
    u8_t idx = 0U;
    bb_edge * copy_edges = edges;
    for (auto & new_edge : new_bb->entry_edges()) {
      if (new_edge.bb == old_bb) {
        continue;
      }
      ::new (copy_edges++) bb_edge{.bb = new_edge.bb, .index = new_edge.index};
      J_ASSERT(new_edge.bb->exit_edges_begin[new_edge.index].bb == new_bb);
      new_edge.bb->exit_edges_begin[new_edge.index].index = idx;
      ++idx;
    }
    for (auto & old_edge : old_bb->entry_edges()) {
      ::new (copy_edges++) bb_edge{.bb = old_edge.bb, .index = old_edge.index};
      J_ASSERT(old_edge.bb->exit_edges_begin[old_edge.index].bb == old_bb);
      old_edge.bb->exit_edges_begin[old_edge.index].index = idx;
      old_edge.bb->exit_edges_begin[old_edge.index].bb = new_bb;
      ++idx;
    }
    new_bb->entry_edges_begin = edges;
    new_bb->num_entry_edges = idx;
    old_bb->num_entry_edges = 0U;
    old_bb->num_exit_edges = 0U;
    --ssa->size;
  }
}
