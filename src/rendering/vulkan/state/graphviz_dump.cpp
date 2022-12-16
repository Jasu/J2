#include "rendering/vulkan/state/graphviz_dump.hpp"

#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/state_iterator.hpp"
#include "rendering/vulkan/state/state_tracker.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"

#include "containers/hash_map.hpp"
#include "files/ofile.hpp"
#include "graphviz/edge.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"
#include "strings/format.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::state {
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace {
    constexpr inline const char * g_error_fill_color = "#E05000";
    constexpr inline const char * g_error_outline_color = "#FFFF00";
    constexpr inline const char * g_error_font_color = "#FFFF00";

    const g::node_attributes g_error_node_attributes{
      a::fill_color = g_error_fill_color,
      a::outline_color = g_error_outline_color,
      a::font_color = g_error_font_color};
    const g::node_attributes g_default_node_attributes{
      a::fill_color = "#FFFFFF",
      a::outline_color = "#000000",
      a::font_color = "#000000"};

    const g::node_attributes g_default_root_node_attributes(
      a::shape = a::node_shape::box,
      a::node_style = a::style::filled
    );

    struct J_TYPE_HIDDEN rec {
      g::graph * main = nullptr;
      g::graph * preconditions = nullptr;
      g::graph * postconditions = nullptr;
    };
  }

  void graphviz_dump(const files::path & path, state_tracker & tracker) {
    g::graph root_graph{
      a::is_directional,
      a::default_node_attributes = g_default_root_node_attributes,
      a::rank_direction = a::rank_dir::left_to_right,
      a::new_rank,
    };

    hash_map<const condition_group *, rec> nodes;

    state_iterator * si = J_MAKE_STATE_ITERATOR(*tracker.dag);
    while (condition_group * n = si->advance_to_next_group()) {
      bool is_duplicate = nodes.contains(n);
      strings::string tooltip_text = strings::format("Node at {}", (void*)n);
      strings::string label_text{strings::format("{}: {} / {} - Level {:08X}", n->name(), n->num_preconditions, n->num_postconditions, n->level)};
      if (strings::string debug_details = n->debug_details(); !debug_details.empty()) {
        label_text += "\n" + debug_details;
      }

      const char * group_color = "#E0E0E0";
      const g::node_attributes * node_attributes = &g_default_node_attributes;

      if (J_UNLIKELY(is_duplicate)) {
        label_text += strings::format("Duplicate node ({})", (void*)n);
        node_attributes = &g_error_node_attributes;
        group_color = "#FFE0A0";
      }

      nodes.emplace(n, rec{
          root_graph.add_subgraph(
            a::rank_direction = a::rank_dir::top_to_bottom,
            a::is_cluster,
            a::label = strings::const_string_view(label_text),
            a::tooltip = strings::const_string_view(tooltip_text),
            a::graph_style = a::style::filled,
            a::foreground_color = group_color,
            a::default_node_attributes = *node_attributes),
        });
    }

    for (auto & var : tracker.dag->variables) {
      g::node * prev = nullptr;
      const state_variable_definition_base * var_def = var.definition;
      J_ASSERT_NOT_NULL(var_def);
      for (auto * n = &tracker.dag->at(var.head); n; n = tracker.dag->next_of(n)) {
        if (!n->group) {
          continue;
        }

        g::graph * parent = nullptr;
        const char *fill_color, *outline_color, *font_color;

        auto group_graph = nodes.maybe_at(n->group);
        if (J_LIKELY(group_graph)) {
          if (n->is_precondition()) {
            fill_color = "#000000";
            outline_color = "#FFFFFF";
            font_color = "#FFFFFF";
            if (!group_graph->preconditions) {
              group_graph->preconditions = group_graph->main->add_subgraph(
                a::label = "Preconditions",
                a::rank_direction = a::rank_dir::top_to_bottom,
                a::label_justify = a::justify::left,
                a::graph_style = a::style::filled,
                a::foreground_color = "#F0F0F0",
                a::is_cluster = true,
                a::rank = a::rank_type::same
              );
            }
            parent = group_graph->preconditions;
          } else {
            fill_color = "#FFFFFF";
            outline_color = "#000000";
            font_color = "#000000";
            if (!group_graph->postconditions) {
              group_graph->postconditions = group_graph->main->add_subgraph(
                a::rank_direction = a::rank_dir::top_to_bottom,
                a::label = "Postconditions",
                a::label_justify = a::justify::left,
                a::graph_style = a::style::filled,
                a::foreground_color = "#BEBEBE",
                a::is_cluster = true,
                a::rank = a::rank_type::same
              );
            }
            parent = group_graph->postconditions;
          }
        } else {
          // Condition does not belong to any iterated group - bug.
          fill_color = g_error_fill_color;
          outline_color = g_error_outline_color;
          font_color = g_error_font_color;
          parent = &root_graph;
        }

        strings::string desc;
        if (n->is_precondition()) {
          desc = var_def->describe_precondition(var.key.wrapper, var.key.index,
                                                *static_cast<precondition_instance*>(n));
        } else {
          desc = var_def->describe_postcondition(var.key.wrapper, var.key.index,
                                                 *static_cast<postcondition_instance*>(n));
        }
        g::node * cond = parent->add_node(
          a::label = strings::const_string_view(desc),
          a::fill_color = fill_color,
          a::outline_color = outline_color,
          a::font_color = font_color
        );
        if (prev) {
          parent->add_edge(prev, cond);
        }
        prev = cond;
      }
    }

    g::serialize(root_graph, mem::make_shared<files::ofile>(path));
  }
}
