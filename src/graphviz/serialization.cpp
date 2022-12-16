#include "graphviz/serialization.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/edge.hpp"
#include "exceptions/assert_lite.hpp"
#include "containers/hash_map.hpp"
#include "strings/find.hpp"
#include "strings/format.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "mem/shared_ptr.hpp"
#include "hzd/ctype.hpp"

namespace j::graphviz {
  namespace {
    using ptr_to_id_map = hash_map<const void *, strings::string>;
    class J_TYPE_HIDDEN serializer {
      strings::formatted_sink m_sink;
      bool m_is_directional;
      bool m_is_first_attribute;
      const char * m_list_prefix;
      u32_t m_id_counter = 0;
      int m_depth = 0;
      strings::string m_indent;
      ptr_to_id_map m_ptr_to_id;

      const strings::string & get_name(strings::const_string_view id_template, const void * J_NOT_NULL item) {
        if (auto id = m_ptr_to_id.maybe_at(item)) {
          return *id;
        }
        return m_ptr_to_id.emplace(item, strings::format(id_template, m_id_counter++)).first->second;
      }

      J_INLINE_GETTER const strings::string & get_node_name(const node * J_NOT_NULL n) {
        return get_name("n{}", n);
      }

      J_INLINE_GETTER const strings::string & get_graph_name(const graph * J_NOT_NULL g) {
        return get_name(g->is_cluster ? "cluster{}" : "G{}", g);
      }

      void write_string(const strings::string & s) {
        if (s.empty()) {
          m_sink.write_unformatted("\"\"");
          return;
        }

        bool had_illegal_character = false;
        for (i32_t i = 0; i < s.size(); ++i) {
          if (!(i == 0 ? is_alpha(s[i]) : is_alnum(s[i])) && s[i] != '_') {
            had_illegal_character = true;
            break;
          }
        }
        if (!had_illegal_character) {
          m_sink.write_unformatted(s);
          return;
        }
        m_sink.write_unformatted("\"");

        u32_t begin = 0;
        for(i32_t end = strings::find_char(s, '"'); end >= 0; begin = end + 1, end = strings::find_char(s, '"', end + 1)) {
          m_sink.write_unformatted(strings::const_string_view(s.data() + begin, end - begin));
          m_sink.write_unformatted("\\\"");
        }
        m_sink.write_unformatted(strings::const_string_view(s.data() + begin, s.size() - begin));
        m_sink.write_unformatted("\"");
      }

      void serialize_edges(const graph & g) {
        bool had_edges = false;
        for (auto & edge : g.edges()) {
          m_sink.write_unformatted(m_indent);
          if (edge.from.node) {
            m_sink.write_unformatted(get_node_name(edge.from.node));
            if (edge.from.port) {
              m_sink.write_unformatted(":");
              m_sink.write_unformatted(edge.from.port);
            }
          } else {
            m_sink.write("{}_dummy_node", get_graph_name(edge.from.subgraph));
          }
          m_sink.write_unformatted(m_is_directional ? " -> " : " -- ");
          if (edge.to.node) {
            m_sink.write_unformatted(get_node_name(edge.to.node));
            if (edge.to.port) {
              m_sink.write_unformatted(":");
              m_sink.write_unformatted(edge.to.port);
            }
          } else {
            m_sink.write("{}_dummy_node", get_graph_name(edge.to.subgraph));
          }
          serialize_attribute_list("", edge.attributes);
          if (edge.to.subgraph) {
            m_sink.write(" [lhead={}]", get_graph_name(edge.to.subgraph));
          }
          if (edge.from.subgraph) {
            m_sink.write(" [ltail={}];\n", get_graph_name(edge.from.subgraph));
          } else {
            m_sink.write_unformatted(";\n");
          }
          had_edges = true;
        }
        if (had_edges) {
          m_sink.write_unformatted("\n");
        }
      }

      void serialize_children(const graph & g) {
        for (auto & ch : g.children()) {
          switch (ch.type) {
          case graph_child_type::node:
          case graph_child_type::secondary_node:
            m_sink.write_unformatted(m_indent);
            m_sink.write_unformatted(get_node_name(ch.node));
            if (ch.type == graph_child_type::node) {
              serialize_attribute_list("", ch.node->attributes);
            }
            m_sink.write_unformatted(";");
            break;
          case graph_child_type::graph:
            serialize_graph("subgraph", *ch.graph, false);
            break;
          }
          m_sink.write_unformatted("\n");
        }
      }

      void serialize_graph(const char * keyword, const graph & g, bool is_root) {
        auto id = get_graph_name(&g);
        m_sink.write("{}{} {} \\{\n", m_indent, keyword, id);
        ++m_depth;
        m_indent = strings::string(2 * m_depth, ' ');

        serialize_graph_attributes(g, is_root);
        if (serialize_attribute_list("node", g.default_node_attributes)) {
          m_sink.write_unformatted(";\n");
        }
        if (serialize_attribute_list("edge", g.default_edge_attributes)) {
          m_sink.write_unformatted(";\n");
        }

        serialize_children(g);
        serialize_edges(g);

        // Add a dummy node so that edges can easily be drawn between subgraphs, even if they
        // do not contain any nodes.
        // m_stream << "\n" << m_indent << id << "_dummy_node [label=\"\", style=invis, width=0, height=0];\n";

        --m_depth;
        m_indent = strings::string(2 * m_depth, ' ');
        m_sink.write("{}\\}\n", m_indent);
      }

      void write_root_attribute(const char * key, const strings::const_string_view & value, bool is_string) {
        m_sink.write("{}{}=", m_indent, key);
        if (is_string) {
          write_string(value);
        } else {
          m_sink.write_unformatted(value);
        }
        m_sink.write_unformatted(";\n");
      }

      void serialize_graph_attributes(const graph & graph, bool is_root) {
        if (is_root) {
          m_sink.write_unformatted("compound=true;\n");
        }
        if (graph.attributes.visit_attributes(attribute_visitor_t::bind<&serializer::write_root_attribute>(this)) || is_root) {
          m_sink.write_unformatted("\n");
        }
      }

      void write_list_attribute(const char * key, const strings::const_string_view & value, bool is_string) {
        if (m_is_first_attribute) {
          m_sink.write("{}{} [", m_indent, m_list_prefix);
          m_is_first_attribute = false;
        } else {
          m_sink.write_unformatted(",");
        }

        m_sink.write("{}=", key);
        if (is_string) {
          write_string(value);
        } else {
          m_sink.write_unformatted(value);
        }
      }

      bool serialize_attribute_list(const char * prefix, const common_attributes & attrs) {
        m_is_first_attribute = true;
        m_list_prefix = prefix;
        if (attrs.visit_attributes(attribute_visitor_t::bind<&serializer::write_list_attribute>(this))) {
          m_sink.write_unformatted("]");
          return true;
        }
        return false;
      }
    public:
      serializer(mem::shared_ptr<streams::sink> sink, bool is_directional)
        : m_sink(static_cast<mem::shared_ptr<streams::sink> &&>(sink)),
          m_is_directional(is_directional)
      {
      }

      void serialize(const graph & graph) {
        serialize_graph(m_is_directional ? "digraph" : "graph", graph, true);
      }
    };
  }

  void serialize(const graph & graph, mem::shared_ptr<streams::sink> sink) {
    serializer(sink, graph.is_directional).serialize(graph);
  }
}
