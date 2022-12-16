#include "lisp/cir/debug/graphviz_dump.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir/debug/op_debug_data.hpp"
#include "lisp/cir//cir_context.hpp"
#include "containers/hash_map.hpp"
#include "containers/vector.hpp"
#include "files/ofile.hpp"
#include "lisp/common/metadata.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"
#include "strings/format.hpp"

namespace j::lisp::cir::inline debug {
  namespace l = lisp;
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace s = strings;

  namespace {
    const g::node_attributes g_default_op_attributes{
      a::shape = a::node_shape::plaintext,
      a::font_name = "Roboto Condensed",
      a::font_size = 11.0,
    };

    struct J_TYPE_HIDDEN bb_rec {
      g::graph * root = nullptr;
      g::graph * main = nullptr;
      g::node * entry = nullptr;
      g::node * exit = nullptr;
    };

    struct J_TYPE_HIDDEN op_rec final {
      g::node * node = nullptr;
      g::graph * rank = nullptr;

      J_BOILERPLATE(op_rec, CTOR_CE)

      op_rec(g::node * J_NOT_NULL node, g::graph * J_NOT_NULL rank) noexcept
        : node(node),
          rank(rank)
      { }
    };

    struct J_TYPE_HIDDEN ctx final {
      g::graph * root = nullptr;
      const cir::op_data_table & op_data;
      hash_map<const bb*, bb_rec> bbs;
      hash_map<const op*, op_rec> ops;
      hash_map<const op*, vector<g::endpoint>> deferred_inputs;
    };
    const colors::rgb8 op_edge_color = colors::rgb8(100,100,100);

    template<typename K, typename V>
    strings::string format_table_row(K key, V value, s::const_string_view bgcolor, bool is_continue = false,
                                     s::const_string_view extra_left = "", s::const_string_view extra_right = "") {
      return strings::format(
        "<tr><td bgcolor=\"{}\" {}{} align=\"left\">{}</td><td bgcolor=\"{}\" {}{} align=\"left\">{}</td></tr>",
        bgcolor,
        is_continue ? "border=\"0\"" : "sides=\"T\"",
        extra_left,
        is_continue ? "" : strings::format("{}", key),
        bgcolor,
        is_continue ? "border=\"0\"" : "sides=\"T\"",
        extra_right,
        value);
    }

    template<typename V>
    strings::string format_table_row_single(V value, s::const_string_view bgcolor) {
      return strings::format(
      "<tr><td colspan=\"2\" bgcolor=\"{}\" sides=\"T\" align=\"left\">{}</td></tr>",
      bgcolor,
      value);
  }

  constexpr const char * input_type_colors[num_input_types]{
    [(u8_t)input_type::none] = "#FFFFC8",
    [(u8_t)input_type::op_result] = "#98C8E0",
    [(u8_t)input_type::reloc] = "#80FFA0",
    [(u8_t)input_type::constant] = "#90E0C8",
    [(u8_t)input_type::mem] = "#F8F8A0",
  };

    strings::string format_inputs(const ctx & ctx, const op & op) {
      strings::string op_rows = "";
      u8_t i = 0U, input_idx = 0U;
      for (auto & input : op.inputs()) {
        const char * name_ptr = ctx.op_data.arg_name(op.type, input_idx);
        strings::string name = name_ptr ? strings::string(name_ptr) : s::format("Arg {}", input_idx);
        const char * color = input_type_colors[(u8_t)input.type];
        if (input.is_mem() || input.is_op_result()) {
          op_rows += format_table_row(name, input, color, false, s::format(" port=\"in{}\"", i), s::format(" port=\"inr{}\"", i));
          ++i;
          if (input.is_mem() && input.mem_data.index.is_op_result()) {
            op_rows += format_table_row("", "Idx", color, true, s::format(" port=\"in{}\"", i), s::format(" port=\"inr{}\"", i));
            ++i;
          }
        } else {
          op_rows += format_table_row(name, input, color);
        }
        ++input_idx;
      }
      return op_rows;
    }

    strings::string format_output(const op & op) {
      if (op.has_result) {
        return format_table_row("Out", op.result.loc_out, "#D890E0", false, "", " port=\"out\"");
      }
      return "";
    }

    /// \todo
    /*
      strings::string format_extra_info(const op & op) {
      auto extra = op.format_extra_info();
      if (!extra) {
      return extra;
      }
      return strings::format("<tr><td colspan=\"2\" bgcolor=\"#F3F3F3\">{}</td></tr>", extra);
      }
    */

    void visit_bb(ctx & c, const bb * b) {
      vector<const char *> barrier_color_list;
      bb_rec rec = bb_rec{
        c.root,
        c.root->add_subgraph(
          a::rank_direction = a::rank_dir::top_to_bottom,
          a::is_cluster,
          a::font_name = "Roboto Condensed",
          a::label = strings::format("#{} {}", b->index, b->name),
          a::graph_style = a::style::striped,
          a::margin = 0.0f,
          a::default_node_attributes = g_default_op_attributes)
      };

      g::graph * cur_rank = nullptr;
      g::graph * prev_rank = nullptr;
      g::node * prev = nullptr;
      const op * prev_op = nullptr;
      barrier_type prev_barrier = barrier_type::none;

      for (const auto & op : *b) {
        const op_debug_info & dbg = op_debug_data[(u8_t)op.type];
        strings::string extra;

        auto md = op.metadata();
        if (auto loc = md.find(metadata_type::source_location)) {
          extra += format_table_row_single(loc->source_location, "#BBAACC");
        }
        if (auto com = md.find(metadata_type::comment)) {
          extra += format_table_row_single(com->text(), "#BBE0D0");
        }

        if (!cur_rank || dbg.barrier_type != prev_barrier) {
          cur_rank = rec.main->add_subgraph(a::rank = a::rank_type::same);
          barrier_color_list.push_back(dbg.barrier_color());
          if (!prev && op.next) {
            cur_rank->add_node(a::label = "", a::width = 2.5f, a::node_style = a::style::invis);
          }
        }
        prev_barrier = dbg.barrier_type;


        g::node * op_node = cur_rank->add_node(
          a::label_is_html,
          a::label = strings::format(
            "<<table port=\"t\" cellspacing=\"0\" border=\"2\" cellborder=\"1\" cellpadding=\"4\">"
            "<tr><td colspan=\"2\" port=\"lbl\" bgcolor=\"black\">{}<font color=\"white\"> {}</font></td></tr>"
            "{}{}{}"
            "</table>>",
            op.has_result ? strings::format("<font point-size=\"8.0\" color=\"#A0A0A0\">#{}</font> ", op.index) : "",
            op.type,
            extra,
            format_inputs(c, op),
            format_output(op)
            /// \todo
            // format_extra_info(op)
            ));


        c.ops.emplace(&op, op_node, cur_rank);

        u32_t index = 0U;
        for (auto & op_res : op.op_results()) {
          auto out_node = c.ops.maybe_at(op_res.use.def);
          const bool is_same_rank = out_node && out_node->rank == cur_rank;
          s::string input_port = s::format(is_same_rank ? "inr{}" : "in{}:w", index);

          ++index;
          if (out_node) {
            rec.main->add_edge(g::endpoint(out_node->node, "out:e"),
                               g::endpoint(op_node, static_cast<s::string&&>(input_port)),
                               a::constraint = false);
          } else {
            c.deferred_inputs.emplace(op_res.use.def, vector<g::endpoint>{}).first->second.emplace_back(
              op_node,
              static_cast<s::string &&>(input_port));
          }
        }

        if (auto def = c.deferred_inputs.maybe_at(&op)) {
          for (auto & port : *def) {
            rec.main->add_edge(g::endpoint(op_node, "out:e"),
                               static_cast<g::endpoint &&>(port),
                               a::constraint = false);
          }
        }

        if (!rec.entry) {
          rec.entry = op_node;
        } else {
          if (prev_rank == cur_rank) {
            // Make the nodes in the current rank run from top to bottom by
            // constraining with an invisible, reversed edge.
            //
            // Reversing the edge with dir fails, due to a GraphViz bug.
            // When there are two edges between the same HTML label but targeting
            // different ports, direction fails, So on ops that consume the output
            // of a previous node, the arrow would be flipped.
            //
            // Using `ordering` did not work at all in this case.
            cur_rank->add_edge(op_node, prev, a::edge_style = a::style::invis);
            cur_rank->add_edge(prev, g::endpoint(op_node, "lbl:n"),
                               a::weight = 0,
                               a::constraint = false,
                               a::edge_color = op_edge_color);
          } else {
            rec.main->add_edge(g::endpoint(prev, "lbl"), g::endpoint(op_node, "lbl"), a::edge_color = op_edge_color);
          }
        }

        prev = op_node;
        prev_op = &op;
        prev_rank = cur_rank;
      }
      rec.exit = prev;
      if (prev_op->previous) {
        cur_rank->add_node(a::label = "", a::width = 2.5f, a::node_style = a::style::invis);
      }

      strings::string color_list;
      bool is_first = true;
      for (auto color : barrier_color_list) {
        if (!is_first) {
          color_list += ":";
        }
        is_first = false;
        color_list += color;
      }
      rec.main->attributes.foreground_color_list = static_cast<strings::string &&>(color_list);
      c.bbs.emplace(b, rec);
    }
  }

  void graphviz_dump(const files::path & path, const cir_ssa & ssa) {
    g::graph root_graph{
      a::concentrate,
      a::is_directional,
      a::default_node_attributes = g_default_op_attributes,
      a::rank_direction = a::rank_dir::left_to_right,
      a::ranksep_equally,
      a::new_rank,
    };

    ctx c = {
      &root_graph,
      cir_context->target.ops,
      {},
      {},
      {},
    };
    for (auto & bb : ssa.in_reverse_postorder()) {
      visit_bb(c, &bb);
    }
    for (auto & to : ssa.in_reverse_postorder()) {
      auto to_rec = c.bbs.maybe_at(&to);
      if (!to_rec) {
        continue;
      }
      for (auto from : to.entry_edges()) {
        auto from_rec = c.bbs.maybe_at(from.bb);
        if (!from_rec) {
          continue;
        }
        J_ASSERT_NOT_NULL(from_rec->exit, from_rec->entry);

        root_graph.add_edge(g::endpoint(from_rec->exit, "lbl"),
                            g::endpoint(to_rec->entry, "lbl"));

      }
    }
    g::serialize(root_graph, j::mem::make_shared<files::ofile>(path));
  }
}
