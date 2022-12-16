#pragma once

#include "strings/regex/char_class.hpp"
#include "strings/string_view.hpp"

namespace j::strings::regex {
  enum node_type : u8_t {
    nt_none,
    nt_literal,
    nt_char_class,
    nt_char_class_negated,
    nt_repetition,
    nt_alternation,

    nt_dot,

    nt_anchor_subject_begin,
    nt_anchor_subject_end,
  };

  struct node_base {
    node_type type;
    node_base *next = nullptr;
  };

  node_base * read(strings::const_string_view str);
  void dump_single(const node_base * node, i32_t indent = 0) noexcept;
  void dump_full(const node_base * node, i32_t indent = 0) noexcept;

  struct node_literal final : node_base {
    i32_t len;
    char char_array[0];

    J_A(AI,ND) inline explicit node_literal(i32_t len) noexcept
      : node_base{nt_literal},
        len(len)
    { }

    J_A(AI,ND,NODISC) inline explicit operator const char*() const noexcept {
      return char_array;
    }

    J_A(AI,ND,NODISC) inline explicit operator strings::const_string_view() const noexcept {
      return {char_array, len};
    }
  };

  struct char_range final {
    /// The first codepoint to match.
    u32_t begin;
    /// One codepoint after the chars to match.
    u32_t end;
    char_range *next;

    J_A(AI,NODISC,ND) inline i32_t num_chars() const noexcept {
      return end - begin;
    }
  };

  struct node_char_class final : node_base {
    char_class value;
    J_A(AI,ND,HIDDEN) inline node_char_class(char_class && value, bool is_negated) noexcept
      : node_base{is_negated ? nt_char_class_negated : nt_char_class},
        value(static_cast<char_class &&>(value))
    { }

    J_A(AI,ND,NODISC) inline bool is_negated() const noexcept {
      return type == nt_char_class_negated;
    }
  };

  struct node_repetition final : node_base {
    i32_t min, max;
    node_base * child;
    J_A(AI,ND,HIDDEN) inline node_repetition(node_base * J_AA(NN) child, i32_t min, i32_t max) noexcept
    : node_base{nt_repetition},
      min(min),
      max(max),
      child(child)
    { }
  };

  struct alternate final {
    node_base * node;
    alternate * next;
  };

  struct node_alternation final : node_base {
    alternate * alternates;
    J_A(AI,ND,HIDDEN) explicit inline node_alternation(alternate * J_AA(NN) alternates) noexcept
      : node_base{nt_alternation},
        alternates(alternates)
    { }
  };
}
