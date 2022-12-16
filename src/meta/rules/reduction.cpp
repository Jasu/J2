#include "reduction.hpp"
#include "strings/format.hpp"
#include "meta/rules/id_resolver.hpp"
#include "meta/rules/dump.hpp"
#include "meta/rules/term_expr.hpp"
#include "logging/global.hpp"
#include "mem/shared_ptr.hpp"
#include "meta/rules/nonterminal.hpp"
#include "algo/quicksort.hpp"

namespace j::meta::inline rules {
  reduction::reduction(reduction && rhs) noexcept = default;
  reduction & reduction::operator=(reduction && rhs) noexcept = default;

  static void replace(const term_expr * J_NOT_NULL old, term_expr * J_NOT_NULL te, void * userdata) noexcept {
    J_ASSERT(old != te && userdata);
    ((reduction_statements*)userdata)->replace(old, te);
  }

  reduction::reduction(const reduction & rhs) noexcept
    : type(rhs.type),
      length(rhs.length),
      depth_difference(rhs.depth_difference),
      size(rhs.size),
      nt(rhs.nt),
      data_type(rhs.data_type),
      loc(rhs.loc),
      statements(rhs.statements)
  {
    for (auto & arg : rhs.args) {
      args.emplace_back(arg, replace, &statements);
    }
  }

  reduction & reduction::operator=(const reduction & rhs) noexcept {
    if (this != &rhs) {
      type = rhs.type;
      length = rhs.length;
      depth_difference = rhs.depth_difference;
      size = rhs.size;
      nt = rhs.nt;
      data_type = rhs.data_type;
      loc = rhs.loc;
      statements = rhs.statements;
      args.clear();
      for (auto & arg : rhs.args) {
        args.emplace_back(arg, replace, &statements);
      }
    }
    return *this;
  }

  void reduction_statements::replace(const term_expr * J_NOT_NULL from, term_expr * J_NOT_NULL to) noexcept {
    for (auto & s : *this) {
      s.replace(from, to);
    }
  }

  reduction::reduction(reduction_type type, reduction_args && args, nt_p nt, i8_t length, source_location loc) noexcept
    : type(type),
      length(length),
      args(static_cast<reduction_args &&>(args)),
      nt(nt),
      loc(loc)
  {
    if (!this->loc && this->args) {
      loc = this->args.front().expr->loc;
    }
  }

  [[nodiscard]] bool reduction::operator==(const reduction & rhs) const noexcept {
    if (type != rhs.type || length != rhs.length || nt != rhs.nt
      || data_type != rhs.data_type || args != rhs.args ||
        depth_difference != rhs.depth_difference || statements.size() != rhs.statements.size()) {
      return false;
    }
    auto r1 = rhs.statements.begin();
    for (const auto & r2 : statements) {
      if (*r2.release_expr != *r1->release_expr || (r2.consuming_te != r1->consuming_te && !r2.consuming_te != !r1->consuming_te && *r2.consuming_te != *r1->consuming_te)) {
        return false;
      }
      ++r1;
    }
    return true;
  }

  [[nodiscard]] pair<i32_t> reduction::extent() const noexcept {
    pair<i32_t> result{0, 0};
    for (auto & a : args) {
      if (a) {
        pair<i32_t> ext = a.expr->extent();
        result.first = j::max(result.first, ext.first);
        result.second = j::min(result.second, ext.second);
      }
    }
    return result;
  }

  void reduction::resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept {
    static_id_resolver id_res;
    if (length < 0) {
      if (!tree.is_single_ctor) {
        length = branch.reduction_depth + 1 + length;
      }
    } else if (!length) {
      using enum reduction_type;
      switch (type) {
      case reduce:
      case reduce_copy:
      case rewrite:
      case erase:
        length = branch.reduction_depth;
        break;
      case truncate:
      case stmt:
      case change_type:
        break;
      case pop: J_FAIL("0-length pop");
      }
    } else if (type == reduction_type::pop && length < 0) {
      length = branch.reduction_depth + 1 + length;
    } else if (type == reduction_type::reduce || type == reduction_type::reduce_copy || type == reduction_type::rewrite || length == -1) {
      length = branch.reduction_depth;
    }
    depth_difference = branch.depth_difference;
    for (auto & a : args) {
      if (a.rewrite_action != rewrite_action::none) {
        if (a.rewrite_offset < 0) {
          a.rewrite_offset = -a.rewrite_offset;
        } else {
          J_ASSERT(a.rewrite_offset < (i32_t)branch.reduction_depth);
          a.rewrite_offset -= branch.reduction_depth - 1;
        }
      }
      a.expr->resolve(branch.caps, scope, &id_res, type == reduction_type::reduce_copy ? term_expr_use::copy : term_expr_use::normal);
    }
  }

  reduction_group::reduction_group(nt_p J_NOT_NULL target_nt, source_location loc) noexcept
    : target_nt(target_nt),
      loc(loc)
  {
  }

  J_A(RNN) reduction * reduction_group::insert_before(reduction * J_NOT_NULL at, reduction && red) noexcept {
    return reds.insert(at, static_cast<reduction &&>(red));
  }

  J_A(RNN) reduction * reduction_group::insert_after(reduction * J_NOT_NULL at, reduction && red) noexcept {
    return reds.insert(at + 1, static_cast<reduction &&>(red));
  }

  reduction & reduction_group::push_back(reduction && red) noexcept {
    return reds.push_back(static_cast<reduction &&>(red));
  }

  void reduction_group::resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept {
    hash = 121;
    i32_t sz = reds.size();
    i32_t i = 0;
    for (auto & r : reds) {
      r.resolve(tree, branch, scope);
      if (++i == sz && r.type == reduction_type::reduce_copy) {
        J_ASSERT(r.args[0]);
        te_type type = r.args[0].expr->get_type(scope);
        J_ASSERT(type.is_any_nt());
        if (!target_nt) {
          J_ASSERT(type.is_nt());
          target_nt = type.nt;
        }
        if (target_nt == type.nt) {
          r.args[0].expr->clear_arg_nt(term_expr_use::copy);
          r.size = 0; // target_nt->data_type ? target_nt->data_type->size : 0;
        }
      }
      hash = crc32(hash + 1, reduction_hash{}(r));
    }
    if (predicate) {
      static_id_resolver id_res;
      predicate->resolve(branch.caps, scope, &id_res);
      hash = crc32(hash + 1, predicate->hash);
    }
  }

  void reduction_group::compute_hash() noexcept {
    hash = 121;
    for (auto & r : reds) {
      hash = crc32(hash + 1, reduction_hash{}(r));
    }
    if (predicate) {
      hash = crc32(hash + 1, predicate->hash);
    }
  }

  [[nodiscard]] bool reduction_group::operator==(const reduction_group & rhs) const noexcept {
    if (reds.size() != rhs.reds.size() || !predicate != !rhs.predicate) {
      return false;
    }
    if (predicate && *predicate != *rhs.predicate) {
      return false;
    }
    auto r2 = rhs.reds.begin();
    for (auto & r : reds) {
      if (r != *r2++) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool reductions::has_default() const noexcept {
    return groups && !groups.back().predicate;
  }

  [[nodiscard]] bool reductions::has_nondefault() const noexcept {
    return groups.size() > 1 || (groups.size() == 1 && !groups.back().predicate);
  }

  [[nodiscard]] pair<i32_t> reductions::extent() const noexcept {
    pair<i32_t> result{0, 0};
    for (auto & g : groups) {
      for (auto & r : g) {
        for (auto & a : r.args) {
          if (a) {
            pair<i32_t> ext = a.expr->extent();
            result.first = j::max(result.first, ext.first);
            result.second = j::min(result.second, ext.second);
          }
        }
      }
      if (g.predicate) {
        pair<i32_t> ext = g.predicate->extent();
        result.first = j::max(result.first, ext.first);
        result.second = j::min(result.second, ext.second);
      }
    }
    return result;
  }

  [[nodiscard]] arg_counts reductions::get_arg_counts() const noexcept {
    arg_counts result;
    for (auto & g : groups) {
      for (auto & r : g) {
        for (auto & a : r.args) {
          result += a.expr->collect_args();
        }
      }
      if (g.predicate) {
        result += g.predicate->collect_args();
      }
    }
    return result;
  }

  [[nodiscard]] bool reductions::operator==(const reductions & rhs) const noexcept {
    if (groups.size() != rhs.groups.size()) {
      return false;
    }
    auto it = rhs.groups.begin();
    for (auto & g : groups) {
      if (g != *it++) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool reductions::is_subset_of(const reductions & rhs) const noexcept {
    const auto *it = groups.begin(), *end = groups.end();
    for (auto & r : rhs.groups) {
      if (r == *it) {
        if (++it == end) {
          return true;
        }
      }
    }
    return false;
  }

  J_A(NI,ND,NODISC,FLATTEN) u32_t reduction_hash::operator()(const reduction & r) const noexcept {
    u32_t result = crc32(1 + r.size, (u32_t)r.length ^ ((u32_t)r.type << 8));
    switch (r.type) {
    case reduction_type::stmt:
    case reduction_type::change_type:
    case reduction_type::erase:
    case reduction_type::truncate:
    case reduction_type::pop:
      break;
    case reduction_type::rewrite:
    case reduction_type::reduce:
      if (r.nt) {
        result = crc32(result, r.nt->index);
      }
      break;
    case reduction_type::reduce_copy:
      if (r.data_type) {
        result ^= crc32(result, r.data_type->name.begin(), r.data_type->name.size());
      }
      break;
    }
    for (auto & a : r.args) {
      result = crc32(13 + result, reduction_value_hash{}(a));
    }
    return result;
  }

  namespace {
    [[nodiscard]] inline i32_t reduction_group_key(const reduction_group & g) {
      return g.predicate ? g.precedence : I32_MAX;
    }
  }

  reductions::reductions() noexcept = default;

  reductions reductions::operator+(const reductions & rhs) const noexcept {
    reductions result{*this};
    for (auto & g : rhs.groups) {
      bool did_find = false;
      bool has_pred = (bool)g.predicate;
      for (auto & g2 : result.groups) {
        if ((!has_pred && !g2.predicate) || g == g2) {
          did_find = true;
          break;
        }
      }
      if (!did_find) {
        result.groups.emplace_back(g);
      }
    }
    algo::quicksort(result.groups.begin(), result.groups.end(), reduction_group_key);
    return result;
  }

  [[nodiscard]] bool reductions::contains_zero_length_reductions() const noexcept {
    for (auto & g : groups) {
      for (auto & r : g) {
        if (r.length == 0) {
          return true;
        }
      }
    }
    return false;
  }

  reduction_group & reductions::push_back(reduction_group && group) noexcept {
    return groups.push_back(static_cast<reduction_group &&>(group));
  }

  void reductions::resolve(const pat_tree & tree, const pat_branch & branch, expr_scope & scope) noexcept {
    hash = 123;
    for (auto & g : groups) {
      g.resolve(tree, branch, scope);
      hash = crc32(hash, g.hash);
    }
  }

  void reductions::compute_hash() noexcept {
    hash = 123;
    for (auto & g : groups) {
      g.compute_hash();
      hash = crc32(hash, g.hash);
    }
  }
  [[nodiscard]] i32_t reduction_group::depth() const noexcept {
    i32_t result = 0;
    for (auto & r : *this) {
      if (r.length < 0) {
        return r.length;
      } else {
        result += r.length;
      }
    }
    return result;
  }

  [[nodiscard]] pair<i32_t> reduction_group::extent() const noexcept {
    pair<i32_t> result{0, 0};
    for (auto & r : *this) {
      pair<i32_t> ext = r.extent();
      result.first = j::max(result.first, ext.first);
      result.second = j::min(result.second, ext.second);
    }
    return result;
  }
}
