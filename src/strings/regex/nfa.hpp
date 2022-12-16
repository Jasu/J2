#pragma once

#include "strings/regex/ast.hpp"
#include "containers/vector.hpp"

namespace j::strings::regex {
  struct nfa_state;

  enum nfa_matcher_type {
    match_none,

    match_epsilon,
    match_anchor_subject_begin,
    match_anchor_subject_end,

    match_ascii_char,
    match_literal,

    match_dot,
    match_char_class,
    match_char_class_negated,
  };

  struct nfa_matcher final {
    nfa_matcher_type matcher;
    union {
      char_class *char_class;
      char ascii_char;
      const_string_view literal;
    } matcher_info{nullptr};
  };

  struct nfa_transition final {
    nfa_matcher match;
    nfa_state *to;
    bool push;
  };

  struct nfa_state final {
    bool accept = false;
    i32_t epsilon_index = -1;
    i32_t backtrack_index = -1;
    vector<nfa_transition> m_transitions{};
    J_A(NODISC,AI) inline i32_t size() const noexcept { return m_transitions.size() - 1; }
    J_A(RNN,NODISC,AI) inline nfa_transition * begin() noexcept { return m_transitions.begin(); }
    J_A(RNN,NODISC,AI) inline const nfa_transition * begin() const noexcept { return m_transitions.begin(); }
    J_A(RNN,NODISC,AI) inline nfa_transition * end() noexcept { return m_transitions.end() - 1; }
    J_A(RNN,NODISC,AI) inline const nfa_transition * end() const noexcept { return m_transitions.end() - 1; }
  };

  struct nfa final {
    J_BOILERPLATE(nfa, COPY_DEL)
    nfa(i32_t num_states, i32_t entry_state) noexcept;
    ~nfa();

    J_A(AI,ND,HIDDEN,NODISC) inline i32_t size() const noexcept { return num_states; }

    J_A(AI,ND,HIDDEN,NODISC) inline nfa_state & operator[](i32_t index) noexcept {
      return states[index];
    }

    J_A(AI,ND,HIDDEN,NODISC) inline const nfa_state & operator[](i32_t index) const noexcept {
      return states[index];
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) inline nfa_state *begin() noexcept {
      return states;
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) inline const nfa_state *begin() const noexcept {
      return states;
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) inline nfa_state *end() noexcept {
      return states + num_states;
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) inline const nfa_state *end() const noexcept {
      return states + num_states;
    }

    i32_t num_states;
    i32_t entry_index;
    i32_t num_epsilon_guards = 0;
    i32_t num_backtrack_states = 0;
    nfa_state states[0];
    J_A(AI,NODISC) inline nfa_state & entry_state() noexcept {
      return states[entry_index];
    }
    J_A(AI,NODISC) inline const nfa_state & entry_state() const noexcept {
      return states[entry_index];
    }
  };

  J_A(RNN,NODISC) nfa * to_nfa(node_base * J_AA(NN) begin);
}
