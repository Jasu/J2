#include "lisp/air/debug/graphviz_dump.hpp"
#include "lisp/air/debug/graphviz_format.hpp"
#include "lisp/air/debug/debug_info_map.hpp"
#include "lisp/common/metadata.hpp"
#include "lisp/air/exprs/accessors.hpp"
#include "lisp/air/exprs/expr_data.hpp"
#include "containers/hash_map.hpp"
#include "containers/ptr_set.hpp"
#include "files/ofile.hpp"
#include "graphviz/edge.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"

namespace j::lisp::air::inline debug {
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace s = strings;

  namespace {
    J_NO_DEBUG const colors::rgb8 control_flow_edge_color{100, 100, 100};

    inline const g::table_style s_dbg{"#E070FF"};

    struct J_TYPE_HIDDEN ctx_graph;

    struct J_TYPE_HIDDEN ctx final {
      g::graph * root = nullptr;
      const debug_info_map * debug_info = nullptr;
      const air_function * fn = nullptr;
      g::graph * parent = nullptr;
      u32_t depth = 0U;
      u32_t rank = 0U;
      ctx_graph * cur_graph = nullptr;
      hash_map<const exprs::expr*, g::node *> nodes{};
      ptr_set<const exprs::expr*> lex_rds = {};
      g::graph * rank_graphs[40] = {nullptr};

      explicit ctx(g::graph * J_NOT_NULL root, const air_function * J_NOT_NULL fn, const debug_info_map * debug_info) noexcept
        : root(root),
          debug_info(debug_info),
          fn(fn),
          parent(root)
      {
      }
    };

    struct J_TYPE_HIDDEN ctx_graph final {
      ctx * c = nullptr;

      g::graph * parent = nullptr;
      g::node * prev_node = nullptr;
      bool is_progn = false;
      bool has_progn = false;

      u32_t rank = 0U;

      ctx_graph * parent_ctx_graph = nullptr;
      g::graph * rank_graph = nullptr;

      ctx_graph() noexcept = default;

      J_RETURNS_NONNULL g::node * add_node(const strings::const_string_view & label) {
        auto node = c->parent->add_node(
          a::label_is_html,
          a::label = label);

        if (!c->rank_graphs[rank]) {
          c->rank_graphs[rank] = c->root->add_subgraph(a::rank = a::rank_type::same);
        }
        c->rank_graphs[rank]->add_secondary_node(node);

        if (prev_node) {
          c->root->add_edge(node, prev_node,
                            a::constraint = false,
                            a::arrow_size = 0.5f,
                            a::arrow_tail = a::arrow::open,
                            a::edge_color = control_flow_edge_color,
                            a::arrow_direction = a::direction::backward);
        }

        return prev_node = node;
      }

      void enter_progn(ctx * J_NOT_NULL c, const exprs::expr * J_NOT_NULL expr, const strings::string & label) noexcept {
        init(c);
        is_progn = true;
        for (auto & in : expr->inputs()) {
          if (in.expr->type == expr_type::fn_body ||
              in.expr->type == expr_type::progn ||
              in.expr->type == expr_type::let) {
            has_progn = true;
            break;
          }
        }
        ++c->depth;

        g::color cluster_color;
        switch (expr->type) {
        case expr_type::progn: cluster_color = (c->depth & 1) ? "#D0D0D0" : "#E0E0E0"; break;
        case expr_type::let: cluster_color = (c->depth & 1) ? "#EAEAD2" : "#F9F9E0"; break;
        case expr_type::fn_body: cluster_color = (c->depth & 1) ? "#CACAF4" : "#DADAFF"; break;
        default:
          J_UNREACHABLE();
        }
        c->parent = parent->add_subgraph(
          a::is_cluster = true,
          a::label_is_html,
          a::label = label,
          a::foreground_color = cluster_color);
      }

      void init(ctx * J_NOT_NULL c) noexcept {
        rank = c->rank;
        this->c = c;
        parent = c->parent;
        parent_ctx_graph = c->cur_graph;
        c->cur_graph = this;
        if (parent_ctx_graph && parent_ctx_graph->is_progn) {
          prev_node = parent_ctx_graph->prev_node;
        }
      }

      void enter_expr(ctx * J_NOT_NULL c) noexcept {
        init(c);
        c->rank++;
        // Workaround node ordering issues by wrapping siblings of progns in progns.
        if (parent_ctx_graph && parent_ctx_graph->has_progn) {
          c->parent = parent->add_subgraph(a::is_cluster = true, a::label = "");
        }
      }

      ~ctx_graph() {
        if (!c) {
          return;
        }
        c->rank = rank;
        if (is_progn) {
          --c->depth;
        }
        c->parent = parent;
        c->cur_graph = parent_ctx_graph;

        if (prev_node && parent_ctx_graph && parent_ctx_graph->is_progn) {
          parent_ctx_graph->prev_node = prev_node;
        }
      }};


    J_RETURNS_NONNULL g::node * visit_expr(ctx & c, const exprs::expr * expr, bool result_used = true) {
      g::table tbl = debug_to_table(c.fn, expr, !result_used);
      if (c.debug_info) {
        for (auto & info : c.debug_info->get(expr)) {
          s::string row[2]{info.name, info.value};
          u32_t offset = (info.name.empty() ? 1U : 0U);
          tbl.push_row(s_dbg, row + offset, 2U - offset);
        }
      }
      s::string table = tbl.render();

      g::node * expr_node = nullptr;
      ctx_graph ctxg;

      const bool is_progn = expr->type == expr_type::fn_body || expr->type == expr_type::progn || expr->type == expr_type::let;
      if (is_progn) {
        ctxg.enter_progn(&c, expr, table);
      } else {
        ctxg.enter_expr(&c);
        expr_node = c.cur_graph->add_node(table);
        if (expr->type == expr_type::lex_wr || expr->type == expr_type::lex_wr_bound
            || expr->type == expr_type::lex_rd || expr->type == expr_type::lex_rd_bound) {
          c.nodes.emplace(expr, expr_node);
          if (expr->type == expr_type::lex_rd || expr->type == expr_type::lex_rd_bound) {
            c.lex_rds.add(expr);
          }
        }
      }

      for (u32_t i = 0; i < expr->num_inputs; ++i) {
        auto expr_result = visit_expr(c, expr->input(i).expr, !is_progn);
        if (!is_progn) {
          c.parent->add_edge(
            g::endpoint(expr_node, strings::format("op{}", i)),
            expr_result,
            a::arrow_tail = a::arrow::normal,
            a::arrow_direction = a::direction::backward,
            a::edge_color = "#4476E6");
        } else {
          expr_node = expr_result;
        }
      }

      if (is_progn && result_used) {
        ctx_graph ctxg2;
        ctxg2.enter_expr(&c);
        return c.cur_graph->add_node(format_result_stub(expr).render());
      }

      return expr_node;
    }
  }

  void graphviz_dump(const files::path & path,
                     const air_function * J_NOT_NULL fn,
                     const debug_info_map * debug_info) {
    g::graph root_graph{
      a::is_directional,
      a::default_node_attributes = g::node_attributes{
        a::shape = a::node_shape::plain,
        a::font_name = "Roboto Condensed",
        a::font_size = 9.0f,
      },
      a::rank_direction = a::rank_dir::left_to_right,
      a::ranksep = 0.55f,
      a::nodesep = 0.11f,
      a::new_rank = true,
      // These are used for graph labels (inherited by subgraphs)
      a::font_name = "Roboto Condensed",
      a::font_size = 9.0f,
      a::label_justify = a::justify::right,
      a::graph_style = a::style::filled,
    };

    {
      ctx c{&root_graph, fn, debug_info};
      visit_expr(c, fn->body);
      for (auto read_op : c.lex_rds) {
        exprs::lex_rd_accessor a{*read_op};
        for (auto reaching_definition : a.reaching_writes()) {
          root_graph.add_edge(
            c.nodes.at(reaching_definition),
            c.nodes.at(read_op),
            a::constraint = false,
            a::arrow_head = a::arrow::open,
            a::edge_color = "#00C828");
        }
      }
    }

    g::serialize(root_graph, j::mem::make_shared<files::ofile>(path));
  }
}
