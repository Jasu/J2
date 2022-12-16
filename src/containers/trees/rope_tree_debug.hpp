#pragma once

#include "logging/global.hpp"
#include "bits/bitops.hpp"
#include "strings/format.hpp"
#include "hzd/concepts.hpp"

namespace j::inline containers::trees {
  template<typename Val>
  concept Formattable = requires (const Val & val) {
    { val.format() } -> SameAs<strings::string>;
  };

  namespace detail {

#ifndef NDEBUG
    template<typename N>
    typename N::metrics_t assert_node(const type_identity_t<N> * parent, const N & node) {
      const u8_t sz = node.size();
      J_ASSERT(parent == node.parent(), "Parent mismatch");
      J_ASSERT(!node.next || &node == node.next->previous, "n->next->prev != n");
      J_ASSERT(!node.previous || &node == node.previous->next, "n->prev->next != n");
      J_ASSERT((!parent && node.is_leaf()) || sz >= N::max_size_v / 2U, "Too small node");
      typename N::metrics_t k{};
      const typename N::controller_t ctrl;
      for (u8_t i = 0U; i < sz; ++i) {
        if (node.is_inner()) {
          J_ASSERT(node.node_at(i).index() == i, "Index mismatch");
        }
        const auto cur_key = node.is_inner() ? node.node_at(i).metrics() : ctrl.compute_metrics(node.value_at(i));
        k += cur_key;
        J_ASSERT(k == node.metrics_at(i), "Key mismatch");
        if (node.is_inner()) {
          const auto sub_key = assert_node(&node, node.node_at(i));
          J_ASSERT(sub_key == cur_key, "Key mismatch 2");
        }
      }
      J_ASSERT(node.empty() || k == node.metrics(), "Total key mismatch");
      return k;
    }
#else
    template<typename N>
    typename N::metrics_t assert_node(const type_identity_t<N> *, const N &) {
      return {};
    }
#endif

    inline constexpr u32_t err_parent    = 1U;
    inline constexpr u32_t err_next_prev = 8U;
    inline constexpr u32_t err_prev_next = 16U;
    inline constexpr u32_t err_size      = 32U;

    template<typename N>
    u32_t validate_node(const N * parent, const N & n) {
      u32_t result = 0U;
      if (parent != n.parent()) {
        result |= err_parent;
      }
      if (n.next && &n != n.next->previous) {
        result |= err_next_prev;
      }
      if (n.previous && &n != n.previous->next) {
        result |= err_prev_next;
      }
      if (n.size() < N::max_size_v / 2 && (n.parent() || n.is_inner())) {
        result |= err_size;
      }
      return result;
    }

    inline const u32_t bar_len = ::j::strlen("│");

    strings::string format_lines(u32_t depth, u32_t lines) {
      strings::string indent(depth ? (depth * 2U + bits::popcount(lines & ((1U << depth) - 1U)) * (bar_len - 1U)) : 0U, ' ');
      char * write_ptr = indent.data();
      for (u32_t i = 0; i < depth; ++i) {
        if (lines & (1U << i)) {
          ::j::memcpy(write_ptr, "│", bar_len);
          write_ptr += bar_len + 1U;
        } else {
          write_ptr += 2;
        }
      }
      return indent;
    }

    template<typename N, typename Fn>
    auto dump_rope_tree_value(const N & n, u32_t depth, u8_t pos, u32_t total_depth, u32_t & lines, Fn && format_value) {
      const char * junction = "├";
      if (pos == 0U) {
        junction = "╭";
      }
      const auto & val = n.value_at(pos);
      J_DEBUG("{}{#bright_yellow}{}{indent:┄}╼{/} {}",
              format_lines(depth, lines),
              junction,
              (total_depth - depth) * 2U + 2U,
              static_cast<Fn &&>(format_value)(val));
      const typename N::controller_t ctrl;
      return ctrl.compute_metrics(val);
    }

    template<typename N>
    void dump_rope_tree_key(const N & n, u32_t depth, u8_t pos, u32_t & lines, const char * line_char, auto k,
                         u32_t errors) {
      const char * junction = "┝";
      const bool is_mid = pos == (n.size() >> 1U) - 1U;
      if (is_mid) {
        junction = "┾";
      }
      if (pos == n.size() - 1U) {
        junction = "╰";
      }

      strings::styled_string key;
      if constexpr (Formattable<typename N::metrics_t>) {
        key = (n.metrics_at(pos) == k)
          ? strings::format_styled("{#bright_green}{}", k.format())
          : strings::format_styled("{#bright_red}{} (expected {})", n.metrics_at(pos).format(), k.format());
      } else {
        key = (n.metrics_at(pos) == k)
          ? strings::format_styled("{#bright_green}{}", k)
          : strings::format_styled("{#bright_red}{} (expected {})", n.metrics_at(pos), k);
      }
      strings::string error_str;
      if (errors & err_parent) {
        error_str += " [Parent mismatch]";
      }
      if (errors & err_next_prev) {
        error_str += " [Next-previous mismatch]";
      }
      if (errors & err_prev_next) {
        error_str += " [Previous-Next mismatch]";
      }
      if (errors & err_size) {
        error_str += " [Node too small]";
      }
      J_DEBUG("{}{}{#white,bold}{}⚿ {}{/}{#bright_red,bold}{} {}{/}",
              format_lines(depth ? depth - 1U : 0, lines),
              line_char,
              junction,
              key,
              error_str,
              is_mid ? strings::format("{}", n.index()) : "");
    }

    template<typename N, typename Fn>
    typename N::metrics_t dump_rope_tree_impl(
      const N & n,
      u32_t depth,
      u8_t pos,
      u32_t total_depth,
      u32_t & lines,
      u32_t errors,
      Fn && format_value
    ) {
      const u8_t sz = n.size();
      typename N::metrics_t k{};
      for (u8_t i = 0U; i < sz; ++i) {
        if (n.is_leaf()) {
          k = k + dump_rope_tree_value(n, depth, i, total_depth, lines, static_cast<Fn &&>(format_value));
        } else {
          k = k + dump_rope_tree_impl(n.node_at(i), depth + 1U, i, total_depth, lines,
                                      validate_node(&n, n.node_at(i)),
                                      static_cast<Fn &&>(format_value));
        }
        const u32_t mask = (1U << depth);
        const u32_t upper_mask = mask >> 1U;
        bool is_median = i == sz / 2U - 1U;

        const char * line_char = (lines & upper_mask) ? "│ " : "  ";

        if (is_median) {
          if (depth) {
            const u8_t parent_sz = ((n.parent()->size() + 1U) & ~1U);
            if (pos == 0U) {
              line_char = "╭─";
              lines |= upper_mask;
            } else if (pos == parent_sz) {
              lines &= ~upper_mask;
              line_char = "╰─";
            } else {
              line_char = "├─";
            }
          }
        }
        if (!depth) {
          line_char = "";
        }

        dump_rope_tree_key(n, depth, i, lines, line_char, k, is_median ? errors : 0U);
      }
      return k;
    }
  }

  template<typename Val>
  strings::string default_format_rope_tree_value(const Val & v) noexcept {
    if constexpr (Formattable<Val>) {
      return v.format();
    } else {
      return strings::format("{}", v);
    }
  }

  template<typename Tree, typename Fn = strings::string (*)(const typename Tree::value_t &)>
  void dump_rope_tree(const Tree & t, const char * title_fmt = "{#yellow_bg,bold,black} Rope tree {/}", Fn && format_node = &default_format_rope_tree_value<typename Tree::value_t>) {
    J_LOG_SCOPED(indent = 1, is_multiline = true, title = strings::format_styled(title_fmt));
    if (t.empty()) {
      J_DEBUG("{#bright_red}Empty tree{/}");
      return;
    }
    u32_t lines = 0;
    detail::dump_rope_tree_impl(t.root, 0, 0, t.root.depth(), lines,
                                detail::validate_node((typename Tree::node_t *)nullptr, t.root),
                                static_cast<Fn &&>(format_node));
  }

  template<typename Tree>
  void assert_rope_tree(const Tree & t) {
    try {
      detail::assert_node(nullptr, t.root);
    } catch (...) {
      dump_rope_tree(t);
      throw;
    }
  }
}

#define J_DUMP_ROPE_TREE(...) ::j::containers::trees::dump_rope_tree(__VA_ARGS__)
#define J_ASSERT_ROPE_TREE(...) ::j::containers::trees::assert_rope_tree(__VA_ARGS__)
