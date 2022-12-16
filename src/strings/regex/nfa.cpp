#include "strings/regex/nfa.hpp"
#include "logging/global.hpp"

namespace j::strings::regex {
  nfa::nfa(i32_t num_states, i32_t entry_state) noexcept
    : num_states(num_states),
      entry_index(entry_state)
  { }

  nfa::~nfa() {
    for (nfa_state & s : *this) {
      s.~nfa_state();
    }
  }

  namespace {
    constexpr inline bool debug_enabled = false;
    struct J_AT(HIDDEN) nfa_state_index {
      i32_t index;

      J_A(AI,ND) inline nfa_state_index(i32_t idx = -1) noexcept : index(idx) { }

      J_A(AI,NODISC,ND) inline bool operator==(const nfa_state_index & rhs) const noexcept = default;
      J_A(AI,NODISC,ND) inline explicit operator bool() const noexcept { return index >= 0; }
      J_A(AI,NODISC,ND) inline bool operator!() const noexcept { return index < 0; }
    };

    struct J_AT(HIDDEN) nfa_build_transition {
      nfa_matcher match;
      nfa_state_index from;
      nfa_state_index to;
      J_A(AI,NODISC,ND) inline bool is_epsilon() const noexcept {
        return match.matcher == match_epsilon;
      }
      J_A(AI) inline nfa_build_transition(nfa_state_index from, nfa_state_index to) noexcept : match{match_epsilon, {}}, from(from), to(to) { }

      template<typename... Ts>
      J_A(AI) inline nfa_build_transition(nfa_state_index from, nfa_state_index to, Ts... match) noexcept : match(match...), from(from), to(to) { }
    };

    struct J_AT(HIDDEN) nfa_build_transitions {
      i32_t num_epsilon = 0;
      vector<nfa_build_transition> vec{};

      J_A(AI,NODISC) inline nfa_build_transition & operator[](i32_t index) noexcept { return vec[index]; }
      J_A(AI,NODISC) inline const nfa_build_transition & operator[](i32_t index) const noexcept { return vec[index]; }

      J_A(AI,NODISC) inline nfa_build_transition *begin() noexcept { return vec.begin(); }
      J_A(AI,NODISC) inline const nfa_build_transition *begin() const noexcept { return vec.begin(); }
      J_A(AI,NODISC) inline nfa_build_transition *end() noexcept { return vec.end(); }
      J_A(AI,NODISC) inline const nfa_build_transition *end() const noexcept { return vec.end(); }

      J_A(AI) inline nfa_build_transition *erase(nfa_build_transition * J_AA(NN) it) noexcept {
        if (it->is_epsilon()) {
          num_epsilon--;
        }
        return vec.erase(it);
      }
      J_A(AI) inline void clear() noexcept { vec.clear(); num_epsilon = 0; }

      J_A(AI,NODISC) inline i32_t size() const noexcept { return vec.size(); }
      J_A(AI,NODISC) inline explicit operator bool() const noexcept { return (bool)vec; }
      J_A(AI,NODISC) inline bool operator!() const noexcept { return !vec; }

      template<typename... Ts>
      nfa_build_transition & add_transition(nfa_state_index from, nfa_state_index to, Ts... ts) noexcept {
        nfa_build_transition & result = vec.emplace_back(from, to, ts...);
        if (result.is_epsilon()) {
          ++num_epsilon;
        }
        return result;
      }

      template<typename... Ts>
      nfa_build_transitions & operator+=(const nfa_build_transitions & rhs) noexcept {
        num_epsilon += rhs.num_epsilon;
        vec.reserve(vec.size() + rhs.size());
        for (auto & t : rhs.vec) {
          vec.push_back(t);
        }
        return *this;
      }
    };

    struct J_AT(HIDDEN) nfa_build_state {
      i32_t num_epsilon_in = 0;
      i32_t num_in = 0;
      i32_t index = 0;
      bool accept = false;
      bool is_pruned = false;
      nfa_build_transitions transitions{};
      i32_t mark = -1;
      J_A(AI,NODISC) inline i32_t num_epsilon_out() const noexcept { return transitions.num_epsilon; }

      J_A(AI,ND) inline nfa_build_state(i32_t index) noexcept
        : index(index)
      { }

      J_A(AI,ND,NODISC) inline bool is_pruneable() const noexcept {
        if (accept) {
          return false;
        }
        if (num_in > 1 && transitions.size() > 1) {
          return false;
        }
        if ((i32_t)transitions.size() > transitions.num_epsilon) {
          return false;
        }
        return true;
      }
    };

    // struct J_AT(HIDDEN) nfa_range {
    //   nfa_state_index begin{}, end{};
    //   J_A(AI,ND) inline nfa_range() noexcept = default;
    //   J_A(AI,ND) inline nfa_range(nfa_state_index begin, nfa_state_index end) noexcept
    //     : begin(begin), end(end)
    //   { }

    //   template<typename... Ts>
    //   J_A(AI,ND,NODISC) inline nfa_range with_begin(Ts... ts) const noexcept {
    //     return {nfa_state_index(ts...), end};
    //   }
    //   template<typename... Ts>
    //   J_A(AI,ND,NODISC) inline nfa_range with_end(Ts... ts) const noexcept {
    //     return {begin, nfa_state_index(ts...)};
    //   }
    //   template<typename... Ts> inline void set_begin(Ts... ts) noexcept { begin = nfa_state_index(ts...); }
    //   template<typename... Ts> inline void set_end(Ts... ts) noexcept { end = nfa_state_index(ts...); }
    // };

    struct J_AT(HIDDEN) nfa_builder {
      vector<nfa_build_state> states{};
      i32_t num_states = 0;
      i32_t depth = 0;
      J_A(AI) inline void enter(const char * J_AA(NN) name) noexcept {
        debug("Entering {}", name);
        ++depth;
      }

      template<typename... Ts>
      void debug(const char * J_AA(NN) msg, const Ts & ... ts) noexcept {
        if (debug_enabled) {
          J_DEBUG_INL("{indent}", depth * 2);
          J_DEBUG(msg, ts...);
        }
      }
      J_A(AI) inline void exit() noexcept {
        --depth;
        debug("Exit");
      }

      nfa_state_index add_state() noexcept {
        i32_t index = states.size();
        states.emplace_back(index);
        ++num_states;
        return index;
      }

      template<typename... Ts>
      inline nfa_state_index add_state(nfa_state_index from, Ts... match) noexcept {
        nfa_state_index index = add_state();
        if (from) {
          add_transition(from, index, match...);
        }
        return index;
      }

      template<typename... Ts>
      void add_transition(nfa_state_index from, nfa_state_index to, Ts... match) noexcept {
        auto & t = states[from.index].transitions.add_transition(from, to, match...);
        if (t.match.matcher == match_epsilon) {
          states[to.index].num_epsilon_in++;
        }
        states[to.index].num_in++;
      }

      J_A(NODISC) bool has_cycle(nfa_state_index start) noexcept {
        states[start.index].mark = -1;
        return has_cycle(start.index, start.index);
      }

      J_A(NODISC) bool has_cycle(i32_t index, i32_t start) noexcept {
        if (states[index].mark == start) {
          return false;
        }
        states[index].mark = start;
        for (auto & t : states[index].transitions) {
          if (t.match.matcher == match_epsilon) {
            if (t.to == start || has_cycle(t.to.index, start)) {
              return true;
            }
          }
        }
        return false;
      }
    };

    struct J_AT(HIDDEN) nfa_component {
      nfa_build_transitions transitions;
      nfa_state_index merged_input, merged_output;

      inline nfa_component() noexcept = default;

      explicit inline nfa_component(const nfa_matcher & m) {
        transitions.add_transition(-1, -1, m);
      }

      nfa_state_index assign_input(nfa_builder & b) {
        if (!merged_input) {
          assign(b.add_state(), -1);
        }
        return merged_input;
      }

      nfa_state_index assign_output(nfa_builder & b) {
        if (!merged_output) {
          assign(-1, b.add_state());
        }
        return merged_output;
      }
      void assign_input(nfa_state_index in) {
        assign(in, -1);
      }
      void assign_output(nfa_state_index out) {
        assign(-1, out);
      }
      void assign(nfa_state_index in, nfa_state_index out) {
        if (in) {
          merged_input = in;
        }
        if (out) {
          merged_output = out;
        }
        for (auto & t : transitions) {
          if (!t.from) {
            t.from = merged_input;
          }
          if (!t.to) {
            t.to = merged_output;
          }
        }
      }

      void resolve(nfa_builder & b) {
        if (!merged_input) {
          merged_input = b.add_state();
        }
        if (!merged_output) {
          merged_output = b.add_state();
        }
        assign(-1, -1);
        for (auto & t : transitions) {
          J_ASSERT(t.to && t.from);
          b.add_transition(t.from, t.to, t.match);
        }
        transitions.clear();
      }

      static void concat(nfa_builder & b, nfa_component & lhs, nfa_component & rhs) {
        if (rhs.merged_input) {
          if (lhs.merged_output) {
            b.add_transition(lhs.merged_output, rhs.merged_input);
          }
          lhs.assign_output(rhs.merged_input);
        } else {
          rhs.assign_input(lhs.assign_output(b));
        }
      }

      nfa_component & append(nfa_builder & b, nfa_component && rhs) {
        concat(b, *this, rhs);
        transitions += rhs.transitions;
        merged_output = rhs.merged_output;
        return *this;
      }
      nfa_component & prepend(nfa_builder & b, nfa_component && rhs) {
        concat(b, rhs, *this);
        rhs.transitions += transitions;
        transitions = rhs.transitions;
        merged_input = rhs.merged_input;
        return *this;
      }
    };

    nfa_matcher to_nfa_matcher(node_literal * J_AA(NN) lit) {
      switch (lit->len) {
      case 0: return {match_epsilon};
      case 1: return {match_ascii_char, {.ascii_char = lit->char_array[0]}};
      default: return {match_literal, {.literal = (const_string_view)*lit}};
      }
    }

    nfa_matcher to_nfa_matcher(node_char_class * J_AA(NN) cls) {
      return {cls->type == nt_char_class_negated ? match_char_class_negated : match_char_class, {.char_class = &cls->value}};
    }

    nfa_matcher special_to_nfa_matcher(node_base * J_AA(NN) special) {
      nfa_matcher_type type;
      switch (special->type) {

      case nt_dot: type = match_dot; break;
      case nt_anchor_subject_begin: type = match_anchor_subject_begin; break;
      case nt_anchor_subject_end: type = match_anchor_subject_end; break;
      default:
        J_FAIL("Unsupported special NFA node");
      }
      return {type};
    }
    nfa_component to_nfa_component(nfa_builder & ctx, node_base * J_AA(NN) node);

    // void set_begin(nfa_builder & ctx, nfa_range & rng, nfa_state_index index) noexcept {
    //   J_ASSERT(index);
    //   if (!rng.begin) {
    //     rng.begin = index;
    //   } else if (rng.begin != index) {
    //     ctx.add_transition(rng.begin, index);
    //   }
    // }

    // void set_end(nfa_builder & ctx, nfa_range & rng, nfa_state_index index) noexcept {
    //   J_ASSERT(index);
    //   if (!rng.end) {
    //     rng.end = index;
    //   } else if (rng.end != index) {
    //     ctx.add_transition(index, rng.end);
    //   }
    // }

    // void set_range(nfa_builder & ctx, nfa_range & rng, nfa_state_index begin, nfa_state_index end) noexcept {
    //   J_ASSERT_NOT_NULL(begin, end);
    //   set_begin(ctx, rng, begin);
    //   set_end(ctx, rng, end);
    // }


    nfa_component alt_to_nfa_component(nfa_builder & ctx, node_alternation * J_AA(NN) alternation) {
      nfa_component result;
      for (alternate *alt = alternation->alternates; alt; alt = alt->next) {
        nfa_component c = to_nfa_component(ctx, alt->node);
        if (c.merged_output) {
          result.transitions.add_transition(c.merged_output, -1);
        }
        if (c.merged_input) {
          result.transitions.add_transition(-1, c.merged_input);
        }
        result.transitions += c.transitions;
      }
      return result;
    }

    nfa_component kleene_to_nfa_component(nfa_builder & b, node_base * J_AA(NN) child) {
      nfa_component result = to_nfa_component(b, child);
      nfa_state_index rep = result.merged_input ? result.merged_input : result.merged_output ? result.merged_output : b.add_state();
      result.assign(rep, rep);
      result.resolve(b);
      result.merged_output = -1;
      result.transitions.add_transition(result.merged_input, -1);
      return result;
    }

    nfa_component plus_to_nfa_component(nfa_builder & b, node_base * J_AA(NN) child) {
      nfa_component result = to_nfa_component(b, child);
      result.resolve(b);
      b.add_transition(result.merged_output, result.merged_input);
      return result;
    }

    nfa_component chain_to_nfa_component(nfa_builder & b, node_base * J_AA(NN) child, i32_t min, i32_t max) {
      J_ASSERT(min >= 0 && max >= 0 && (min || max) || (!max || max >= min));
      nfa_component result;
      for (i32_t i = max ? max : min; i; --i) {
        result.prepend(b, to_nfa_component(b, child));
        if (i > min) {
          result.transitions.add_transition(-1, -1);
        } else if (i == min && !max) {
          result.resolve(b);
          b.add_transition(result.merged_output, result.merged_input);
        }
      }
      return result;
    }

    nfa_component rep_to_nfa_component(nfa_builder & b, node_repetition * J_AA(NN) rep) {
      i32_t min = rep->min, max = rep->max;
      if (!max) {
        if (!min) {
          return kleene_to_nfa_component(b, rep->child);
        } else if (min == 1) {
          return plus_to_nfa_component(b, rep->child);
        }
      }
      return chain_to_nfa_component(b, rep->child, min, max);
    }

    nfa_component to_nfa_component_single(nfa_builder & b, node_base * J_AA(NN) node) {
      switch (node->type) {
      case nt_none: J_FAIL("None type");

      case nt_repetition:
        b.enter("rep");
        return rep_to_nfa_component(b, (node_repetition*)node);
      case nt_alternation:
        b.enter("alt");
        return alt_to_nfa_component(b, (node_alternation*)node);

      case nt_literal:
        b.enter("lit");
        return nfa_component{to_nfa_matcher((node_literal*)node)};

      case nt_char_class_negated:
      case nt_char_class:
        b.enter("char class");
        return nfa_component{to_nfa_matcher((node_char_class*)node)};

      case nt_anchor_subject_begin:
      case nt_anchor_subject_end:
      case nt_dot:

        b.enter("special");
        return nfa_component{special_to_nfa_matcher(node)};
      }
    }

    nfa_component to_nfa_component(nfa_builder & b, node_base * J_AA(NN) node) {
      b.enter("Seq");
      nfa_component result = to_nfa_component_single(b, node);
      b.exit();
      for (node_base *it = node->next; it; it = it->next) {
        result.append(b, to_nfa_component_single(b, it));
        b.exit();
      }
      b.exit();
      return result;
    }
  }


  J_A(RNN,NODISC) nfa * to_nfa(node_base * J_AA(NN) begin) {
    nfa_builder b{};
    nfa_component c = to_nfa_component(b, begin);
    c.resolve(b);
    nfa_state_index init_state = c.merged_input, exit_state = c.merged_output;
    J_ASSERT_NOT_NULL(init_state, exit_state);
    if (b.states[exit_state.index].transitions) {
      exit_state = b.add_state(exit_state);
    }
    b.states[exit_state.index].accept = true;
    // ctx.prune();
    i32_t sz = b.states.size();
    i32_t indices[sz];

    i32_t cur_idx = 0;
    for (i32_t i = 0; i < sz; ++i) {
      indices[i] = b.states[i].is_pruned ? -1 : cur_idx++;
    }
    J_ASSERT(cur_idx == b.num_states);

    nfa * result = ::new (j::allocate(sizeof(nfa) + (cur_idx * sizeof(nfa_state)))) nfa(cur_idx, indices[init_state.index]);
    nfa_state * states = result->begin();
    for (i32_t i = 0, out_idx = 0; i != sz; ++i) {
      nfa_build_state & in = b.states[i];
      if (in.is_pruned) {
        continue;
      }
      i32_t epsilon_idx = -1;
      i32_t backtrack_idx = -1;
      if (b.has_cycle(i)) {
        epsilon_idx = result->num_epsilon_guards++;
      }
      if (in.transitions.size() > 1) {
        backtrack_idx = result->num_backtrack_states++;
      }
      nfa_state & out = *::new (states + out_idx) nfa_state{in.accept, epsilon_idx, backtrack_idx, {}};
      for (auto & trans : in.transitions) {
        out.m_transitions.push_back({ trans.match, states + indices[trans.to.index], backtrack_idx >= 0 });
      }
      out.m_transitions.push_back({ {match_none}, nullptr, false });
      out_idx++;
    }
    return result;
  }
}
