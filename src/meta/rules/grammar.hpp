#pragma once

#include "meta/rules/ambient_patterns.hpp"
#include "meta/rules/reduction.hpp"

namespace j::meta::inline rules {
  using reds_hash_set_t = hash_set<red_p, reductions_hash, reductions_hash, reductions_const_key>;
  struct grammar final {
    grammar(module * J_NOT_NULL mod) noexcept;

    terminal_collection terminals{};
    module * mod = nullptr;
    nt_p any = nullptr;
    nt_p anys = nullptr;
    ambient_patterns ambient_pats;
    reds_hash_set_t reductions{};
    J_A(NODISC,RNN) nt_p get_nt(strings::const_string_view name) const noexcept;

    [[nodiscard]] pat_p add_reduction(struct reductions && r, pat_precedence precedence);
    [[nodiscard]] pat_p add_reduction(reduction && r, pat_precedence precedence);
    [[nodiscard]] red_p add_reduction_raw(reduction && r);
    [[nodiscard]] red_p add_reduction_raw(struct reductions && r);
  };

  [[nodiscard]] grammar * make_grammar(module & mod);

  void compute_first(grammar & g);
}
J_DECLARE_EXTERN_HASH_SET(j::mem::shared_ptr<j::meta::rules::reductions>, HASH(j::meta::rules::reductions_hash), KEYEQ(j::meta::rules::reductions_hash), CONST_KEY(j::meta::reductions_const_key));
