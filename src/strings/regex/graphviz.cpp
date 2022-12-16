#include "strings/regex/graphviz.hpp"
#include "strings/unicode/utf8.hpp"
#include "files/ofile.hpp"
#include "graphviz/edge.hpp"
#include "strings/format.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"

namespace j::strings::regex {
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace s = strings;
  namespace {
    struct J_AT(HIDDEN) context {
      g::graph graph;
      const nfa * nfa;
      vector<g::node*> state_nodes;

      i32_t state_index(const nfa_state * J_AA(NN) s) const noexcept {
        return s - nfa->begin();
      }
    };

    g::node * dump_state(context & ctx, const nfa_state * J_AA(NN) state) {
      g::node * result;
      s::string name;
      if (state->backtrack_index >= 0) {
        name = s::format("{}", state->backtrack_index);
      }
      if (state->epsilon_index >= 0) {
        name += s::format(":{}", state->epsilon_index);
      }
      if (state->accept) {
        result = ctx.graph.add_node(a::label = name, a::shape = a::node_shape::doublecircle);
      } else if (name) {
        result = ctx.graph.add_node(a::label = name);
      } else {
        result = ctx.graph.add_node();
      }
      if (state == &ctx.nfa->entry_state()) {
        ctx.graph.add_edge(
          ctx.graph.add_node(a::shape = a::node_shape::none),
          result
        );
      }
      return result;
    }

    s::string matcher_label(const nfa_matcher & m) noexcept {
        switch(m.matcher) {
        case match_none: return "NONE";
        case match_epsilon: return "ɛ";
        case match_dot: return "·";
        case match_anchor_subject_begin: return "^";
        case match_anchor_subject_end: return "$";
        case match_ascii_char:
          return s::format("'{}' #{}", m.matcher_info.ascii_char, (i32_t)m.matcher_info.ascii_char);
        case match_literal:
          return m.matcher_info.literal;
        case match_char_class:
        case match_char_class_negated: {
          s::string result = m.matcher == match_char_class_negated ? "Not " : "";
          // const node_char_class* n = m.matcher_info.char_class;
          // if (n->ascii_mask) {
          //   result += "Mask: ";
          //   u128_t mask = 1;
          //   for (i32_t i = 0; i < 128; ++i, mask <<= 1) {
          //     if (n->ascii_mask & mask) {
          //       result += s::format("{}", (char)i);
          //     }
          //   }
          // }
          // for (auto p = n->ranges; p; p = p->next) {
          //   char begin[5], end[5];
          //   *utf8_encode(begin, p->begin) = 0;
          //   *utf8_encode(end, p->end - 1) = 0;
          //   if (p->num_chars() == 1) {
          //     result += s::format("U#{} '{}'", p->begin, (const char*)begin);
          //   } else {
          //     result += s::format("U#{}-U#{} '{}'-'{}'", p->begin, p->end, (const char*)begin, (const char*)end);
          //   }
          // }
          return result;
        }
        }
    }

    void dump_state_edges(context & ctx, const nfa_state * J_AA(NN) state) {
      g::node *from = ctx.state_nodes[ctx.state_index(state)];
      for (const nfa_transition & transition : *state) {
        g::node *to = ctx.state_nodes[ctx.state_index(transition.to)];
        if (transition.match.matcher == match_epsilon) {
          const char * color = transition.push ? "#2070F2" : "#8090E9";
          ctx.graph.add_edge(from, to, a::font_color = color, a::edge_color = color, a::label = "ɛ");
        } else {
          const char * color = transition.push ? "#000000" : "";
          ctx.graph.add_edge(from, to, a::font_color = color, a::edge_color = color, a::label = matcher_label(transition.match));
        }
      }
    }
  }

  void graphviz_dump(const files::path & path, const_string_view regex, const nfa * J_AA(NN) nfa) {
    context ctx{
      g::graph{
        a::is_directional,
        a::label = regex,
        a::label_align = a::align::top,
        a::font_name = "Roboto",
        a::font_size = 10.0f,
        a::default_edge_attributes = g::edge_attributes{
          a::font_name = "Roboto",
          a::font_size = 10.0f,
          a::edge_color = "#666666",
          a::font_color = "#666666",
        },
        a::default_node_attributes = g::node_attributes{
          a::label = "",
          a::shape = a::node_shape::circle,
          a::font_name = "Roboto",
          a::font_size = 10.0f,
        },
        a::ordering = a::order::out,
        a::rank_direction = a::rank_dir::left_to_right,
      },
      nfa,
      {},
    };

    for (auto & state : *nfa) {
      ctx.state_nodes.push_back(dump_state(ctx, &state));
    }
    for (auto & state : *nfa) {
      dump_state_edges(ctx, &state);
    }

    g::serialize(ctx.graph, j::mem::make_shared<files::ofile>(path));
  }
}
