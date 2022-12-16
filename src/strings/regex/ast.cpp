#include "strings/regex/ast.hpp"
#include "logging/global.hpp"
#include "strings/unicode/utf8.hpp"

namespace j::strings::regex {
  void dump_full(const node_base * node, i32_t indent) noexcept {
    for (; node; node = node->next) {
      dump_single(node, indent);
    }
  }
  void dump_single(const node_base * J_AA(NN) node, i32_t indent) noexcept {
    J_DEBUG_INL("{indent}", indent);
    switch (node->type) {
    case nt_none: J_DEBUG("{#error}[Empty node]{/}"); return;
    case nt_literal: {
      const node_literal * n = static_cast<const node_literal*>(node);
      if (!n->len) {
        J_DEBUG("{#bright_yellow,bold}ɛ{/}");
      } else {
        J_DEBUG("{#bold}Literal: {/}{#light_gray,italic}\"{/}{}{#light_gray,italic}\"{/}", (const char*)*n);
      }
      return;
    }
    case nt_char_class_negated:
    case nt_char_class: {
      J_DEBUG_INL(node->type == nt_char_class ? "{#bright_cyan,bold}Char class{/}" : "{#bright_red,bold}Negated char class{/}");
      const char_class & cc = static_cast<const node_char_class*>(node)->value;
      if (cc.ascii_mask) {
        J_DEBUG_INL(" {#light_gray}{}{/}", format_mask(cc.ascii_mask));
      }
      if (cc.utf8_tree) {
        J_DEBUG_INL(" {#bright_magenta}{}{/}", format_char_tree(cc.utf8_tree));
      }
      J_DEBUG("");
      break;
    }
    case nt_repetition: {
      const node_repetition * n = static_cast<const node_repetition*>(node);
      if (!n->max && !n->min) {
        J_DEBUG("{indent}{#bright_yellow,bold}Repetition: Kleene (*):{/}", indent);
      } else if (!n->max && n->min == 1) {
        J_DEBUG("{indent}{#bright_green,bold}Repetition: One or more (+):{/}", indent);
      } else if (n->max == 1 && !n->min) {
        J_DEBUG("{indent}{#bright_cyan,bold}Optional (?):{/}", indent);
      } else if (n->max == n->min) {
        J_DEBUG("{indent}{#bright_green,bold}Repetition: Count \\{{}}:", indent, n->min);
      } else {
        J_DEBUG("{indent}{#bright_green,bold}Repetition: Range \\{{},{}}:", indent, n->min, n->max);
      }
      dump_full(n->child, indent + 4);
      break;
    }
    case nt_dot:
      J_DEBUG("{indent}{#green,bold}Non-newline (.){/}", indent);
      break;
    case nt_alternation: {
      J_DEBUG("{indent}{#bright_magenta,bold}Alternation:{/}", indent);
      const node_alternation * n = static_cast<const node_alternation*>(node);
      for (auto p = n->alternates; p; p = p->next) {
        dump_full(p->node, indent + 4);
      }
      break;
    }
    case nt_anchor_subject_begin:
      J_DEBUG("{indent}{#bright_green,bold}Subject start anchor (^){/}", indent);
      break;
    case nt_anchor_subject_end:
      J_DEBUG("{indent}{#red,bold}Subject end anchor ($){/}", indent);
      break;
    }
  }
}
