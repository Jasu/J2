#pragma once

#include "logging/global.hpp"
#include "bits/bitops.hpp"
#include "strings/format.hpp"
#include "exceptions/assert.hpp"

namespace j::inline containers::trees {
  namespace detail {
    template<typename N, typename Fn>
    void dump_avl_impl(const N * n, u32_t depth, int pos, u32_t total_depth, u32_t & lines, Fn && format_value) {
      if (!n) {
        return;
      }

      if (n->left) {
        dump_avl_impl(static_cast<const N *>(n->left), depth + 1, -1, total_depth, lines, static_cast<Fn &&>(format_value));
      }

      lines &= ~(1U << depth);
      if (pos > 0) {
        lines &= ~(1U << (depth - 1U));
      }

      const u32_t bar_len = ::j::strlen("│");
      strings::string indent(depth ? (depth + bits::popcount(lines) * (bar_len - 1U) - 1U) : 0U, ' ');
      char * write_ptr = indent.data();
      for (u32_t i = 0; i < depth; ++i) {
        if (lines & (1U << i)) {
          ::j::memcpy(write_ptr, "│", bar_len);
          write_ptr += bar_len;
        } else {
          ++write_ptr;
        }
      }

      u32_t style = 0;
      if (pos == 0) {
        style = 1;
      } else if (!n->left && !n->right) {
        style = 2;
      }
      const int bf = (n->right ? n->right->depth() : 0) - (n->left ? n->left->depth() : 0);

      J_DEBUG("{#gray_bg,bright_green}{indent}{}{#[bright_yellow,light_gray,bright_red]}{}{/}{indent}{#black}▐{/}{/}"
              "{#[white,bright_magenta,gray]}{}{}{}{indent:┄}╼{/} "
              "{:4} "
              "{#[bright_yellow,gray,bright_cyan]}{}{/}  "
              "LVL: {} "
              "Depth: {} "
              "{#[bright_yellow,gray,bright_cyan,bright_red]}Real BF: {:2}{/} {}",
              depth,
              pos ? (pos > 0 ? "▝" : "▗") : "▐",
              n->node_data,
              n->left ? (n->right ? "▌" : "▘") : (n->right ? "▖" : " "),
              total_depth - depth - 1,

              style,
              indent,
              pos == 0 ? "" : (pos > 0 ? "╰" : "╭"),
              pos == 0
                ? (n->left ? (n->right ? "┽" : "┵") : (n->right ? "┭" : "┄"))
                : (n->left ? (n->right ? "┼" : "┴") : (n->right ? "┬" : "┄")),
              (total_depth - depth) - 1,

              n->key,

              n->node_data,
              n->node_data == 1 ? "Balanced ⯐" : (n->node_data == 2 ? "R-heavy  ⮩" : "L-heavy  ⮪"),

              depth,
              n->depth(),

              (bf < -1 || bf > 1 || bf + 1 != n->node_data) ? 3 : bf + 1,
              bf,
              static_cast<Fn &&>(format_value)(*n));

      if (pos < 0) {
        lines |= (1U << (depth - 1U));
      }
      if (n->right) {
        lines |= 1U << depth;
        dump_avl_impl(static_cast<const N *>(n->right), depth + 1, 1, total_depth, lines, static_cast<Fn &&>(format_value));
      }
    }

#ifndef NDEBUG
    template<typename N>
    int assert_avl_impl(N * n) {
      if (!n) {
        return 0;
      }
      const int left_depth = assert_avl_impl(n->left),
        right_depth = assert_avl_impl(n->right);
      const int balance = right_depth - left_depth;
      J_ASSERT(balance >= -1 && balance <= 1, "Real balance out of range: L depth = {}, R depth = {}", left_depth, right_depth);
      J_ASSERT(n->node_data == balance + 1, "Stored balance did not match L={}, R={}, B={} Stored={}", left_depth, right_depth, balance, (int)n->node_data - 1);
      return j::max(left_depth, right_depth) + 1;
    }
#else
    template<typename N> int assert_avl_impl(N *)
    { return 0; }
#endif
  }

  template<typename Tree>
  const char * default_format_value(const typename Tree::node_t &) noexcept {
    return "";
  }

  template<typename Tree, typename Fn = const char * (*)(const typename Tree::node_t &)>
  void dump_avl(const Tree & t, const char * title_fmt = "{#yellow_bg,bold,black} AVL Tree {/}", Fn && format_node = &default_format_value<Tree>) {
    J_LOG_SCOPED(indent = 1, is_multiline = true, title = strings::format_styled(title_fmt));
    if (t.empty()) {
      J_DEBUG("{#bright_red}Empty tree{/}");
      return;
    }
    u32_t lines = 0;
    detail::dump_avl_impl(t.root(), 0, 0, t.root()->depth(), lines, static_cast<Fn &&>(format_node));
  }

  template<typename Tree>
  void assert_avl(const Tree & t) {
    try {
      detail::assert_avl_impl(t.root());
    } catch (...) {
      dump_avl(t);
      throw;
    }
  }
}

#ifndef NDEBUG
#define J_ASSERT_AVL(...) ::j::containers::trees::assert_avl(__VA_ARGS__)
#else
#define J_ASSERT_AVL(...) do { } while (false)
#endif

#define J_DUMP_AVL(...) ::j::containers::trees::dump_avl(__VA_ARGS__)
