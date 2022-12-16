#pragma once

#include "logging/global.hpp"
#include "hzd/mem.hpp"
#include "strings/regex/nfa.hpp"

namespace j::strings::regex {
  template<typename It>
  struct nfa_match_backtrack final {
    It input;
    const nfa_transition * cur_transition = nullptr;
    u64_t visited_epsilons = 0;
  };

  template<typename It>
  struct nfa_match {
    It begin{}, end{};
    J_A(AI) inline explicit operator bool() const noexcept { return begin; }
    J_A(AI) inline bool operator!() const noexcept { return !begin; }
  };

  template<typename It>
  struct nfa_match<It*> {
    It *begin = nullptr, *end = nullptr;
    J_A(AI) inline explicit operator bool() const noexcept { return begin; }
    J_A(AI) inline bool operator!() const noexcept { return !begin; }
  };

  template<typename It>
  struct nfa_match_stack {
    J_BOILERPLATE(nfa_match_stack, COPY_DEL)

    nfa_match_backtrack<It> *it = nullptr;
    nfa_match_backtrack<It> *end = nullptr;
    nfa_match_backtrack<It> *begin = nullptr;

    nfa_match_stack() noexcept {
      realloc(32);
    }

    ~nfa_match_stack() {
      j::free(begin);
    }
    J_A(RNN,NODISC) nfa_match_backtrack<It>* operator->() noexcept { J_ASSERT(it >= begin); return it; }

    void realloc(i32_t num) {
      i32_t idx = it - begin;
      J_ASSERT(idx >= 0 && num > idx);
      begin = (nfa_match_backtrack<It>*)j::reallocate(begin, num * sizeof(nfa_match_backtrack<It>));
      it = begin + idx;
      end = begin + num;
    }

    void push() noexcept {
      ++it;
      if (it == end) {
        realloc(end - begin + j::clamp(32, end - begin, 1024));
      }
    }

    J_A(AI,HIDDEN,ND,NODISC) inline bool operator!() const noexcept {
      return it == begin;
    }
    J_A(AI,HIDDEN,ND,NODISC) explicit inline operator bool() const noexcept {
      return it != begin;
    }

    bool pop() noexcept {
      if (it == begin) { return false; }
      J_ASSERT(it > begin);
      --it;
      return true;
    }
  };


  template<typename It>
  struct regex_matcher {
    It it, end, begin;
    const nfa_transition * cur_trans;
    nfa_match_stack<It> stack{};

    regex_matcher(const nfa* J_AA(NN) nfa, It begin, It end)
      : it(begin), end(end), begin(begin)
    {
      stack->input = begin;
      cur_trans = stack->cur_transition = nfa->entry_state().begin();
      stack->visited_epsilons = 0;
    }

    void enter_state(const nfa_transition * J_AA(NN) trans, u64_t visited_epsilons) {
      auto state = trans->to;
      J_DEBUG("Entering{} Ch#{} V{:08b}", trans->push ? " (push)" : "", (i32_t)(it - begin), visited_epsilons);
      stack->cur_transition = cur_trans;
      if (trans->push) {
        stack.push();
      }
      stack->input = it;
      stack->cur_transition = state->begin();
      stack->visited_epsilons = visited_epsilons;
      cur_trans = state->begin();
    }

    bool pop() {
      if (!stack.pop()) {
        return false;
      }
      it = stack->input;
      cur_trans = stack->cur_transition + 1;
      return true;
    }

    bool enter(const nfa_transition * J_AA(NN) trans) {
      switch (trans->match.matcher) {
      case match_epsilon: {
        u64_t eps = stack->visited_epsilons;
        if (trans->to->epsilon_index >= 0) {
          eps |= 1UL << trans->to->epsilon_index;
        }
        enter_state(trans, eps);
        break;
      }
      case match_ascii_char:
      case match_dot:
      case match_char_class:
      case match_char_class_negated:
        ++it;
        enter_state(trans, 0);
        break;
      case match_literal:
        it += trans->match.matcher_info.literal.size();
        enter_state(trans, 0);
        break;
      case match_anchor_subject_end:
      case match_anchor_subject_begin: J_FAIL("Todo");
      case match_none: J_FAIL("None");
      }
      return trans->to->accept;
    }

    bool matches(const nfa_transition * J_AA(NN) trans) noexcept {
      switch (trans->match.matcher) {
      case match_epsilon:
        return trans->to->epsilon_index < 0 || !(stack->visited_epsilons & (1UL << trans->to->epsilon_index));
      case match_ascii_char:
        return *it == trans->match.matcher_info.ascii_char;
      case match_dot:
        return *it && *it == '\r' && *it != '\n';
      case match_char_class: {
        return trans->match.matcher_info.char_class->ascii_mask.has(*it);
      }
      case match_char_class_negated: {
        return !trans->match.matcher_info.char_class->ascii_mask.has(*it);
      }
      case match_literal: {
        i32_t sz = trans->match.matcher_info.literal.size();
        return end - it >= sz && !j::memcmp(it, trans->match.matcher_info.literal.begin(), sz);
      }
      case match_anchor_subject_end:
      case match_anchor_subject_begin:
         J_FAIL("Todo");
      case match_none:
        return false;
      }
    }

    nfa_match<It> match() {
      for (;;) {
        if (matches(cur_trans)) {
          J_DEBUG("Matches");
          if (enter(cur_trans)) {
            J_DEBUG("Matched");
            return {begin, it};
          }
        } else if (cur_trans->match.matcher != match_none) {
          J_DEBUG("Next");
          ++cur_trans;
        } else if (pop()) {
          J_DEBUG("Pop");
        } else {
          break;
        }
      }
      return {};
    }
  };
}
