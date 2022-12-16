#pragma once

#include "containers/vector.hpp"
#include "meta/comments.hpp"
#include "meta/rules/reduction.hpp"
#include "meta/basic_node_set.hpp"

namespace j::meta {
  struct expr;
}
namespace j::meta::inline rules {
  struct rule final {
    pat_tree * match = nullptr;
    reductions reds;
    pat_layer layer = pat_layer::main;
    i8_t precedence = 0;
    bool has_parent = false;
    bool not_at_exit = false;
    bool debug = false;
    source_location loc{};
    strings::string name;

    basic_node_set parent_terms{};

    J_A(AI,ND) inline rule() noexcept = default;

    J_A(AI,ND) explicit inline rule(source_location loc) noexcept
      : loc(loc)
    { }

    J_A(AI) inline rule(pat_tree * J_NOT_NULL match, pat_layer layer, i8_t precedence, bool debug, source_location loc) noexcept
      : match(match),
        layer(layer),
        precedence(precedence),
        debug(debug),
        loc(loc)
    { }

    J_A(AI,ND) void push_reduction_group(reduction_group && group) noexcept {
      reds.push_back(static_cast<reduction_group &&>(group));
    }
  };
}
