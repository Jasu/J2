#pragma once

#include "meta/dump.hpp"
#include "meta/rules/common.hpp"
#include "containers/vector.hpp"

namespace j::meta::inline rules {
  struct grammar;
  struct reduction;
  struct pat_paths;
  struct terminal_set;
  struct pat_elem;
  using pat_elem_vec = vector<pat_elem>;

  void dump(dump_context & ctx, const reduction & r) noexcept;
  void dump(dump_context & ctx, const reduction_group & g) noexcept;
  void dump(dump_context & ctx, const red_p & r) noexcept;
  void dump(dump_context & ctx, const grammar & g, const pat_elem_vec & p) noexcept;
  void dump(dump_context & ctx, const grammar & g, const pat_paths & paths) noexcept;

  void dump(dump_context & ctx, const grammar & g, const pat_p & p) noexcept;
  void dump(dump_context & ctx, const grammar & g, nt_p nt) noexcept;
  void dump(dump_context & ctx, const grammar & g, const terminal_set & ls) noexcept;

  void debug_dump_pat(const grammar & g, const pat_p & p) noexcept;
}
