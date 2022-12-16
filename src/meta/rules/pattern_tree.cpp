#include "pattern_tree.hpp"
#include "meta/term.hpp"
#include "logging/global.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "algo/quicksort.hpp"
#include "meta/module.hpp"
#include "mem/shared_ptr.hpp"
#include "meta/rules/grammar.hpp"
#include "meta/rules/reduction.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/nonterminal.hpp"

J_DEFINE_EXTERN_HASH_SET(j::meta::pat_elem_vec, HASH(j::meta::pat_elem_vec_hash), KEYEQ(j::meta::pat_elem_vec_hash));

namespace j::meta::inline rules {
  pat_paths::pat_paths(hash_set<pat_elem_vec, pat_elem_vec_hash, pat_elem_vec_hash> && vecs) noexcept
    : vecs(static_cast<hash_set<pat_elem_vec, pat_elem_vec_hash, pat_elem_vec_hash> &&>(vecs))
  { }

  pat_p add_alternate(pat_p && to, pat_p alt) noexcept {
    J_ASSERT_NOT_NULL(alt);
    if (!to) {
      return static_cast<mem::shared_ptr<pat>&&>(alt);
    }
    if (to == alt || *to == *alt) {
      return static_cast<mem::shared_ptr<pat>&&>(to);
    }
    if (to->type != pat_type::alternation) {
      to = mem::make_shared<pat>(pat_alternation_v, static_cast<pat_p &&>(to), to->precedence);
    }

    if (alt->type == pat_type::alternation) {
      J_ASSERT(to != alt);
      for (auto & a : alt->alternates) {
        J_ASSERT_NOT_NULL(a);
        to = add_alternate(static_cast<pat_p &&>(to), a);
      }
      return static_cast<pat_p&&>(to);
    }
    auto it = to->alternates.begin(), e = to->alternates.end();
    for (; it != e && alt->precedence >= (*it)->precedence; ++it) {
      if (*it == alt || **it == *alt) {
        return static_cast<mem::shared_ptr<pat>&&>(to);
      }
    }
    to->alternates.emplace(it, alt);
    to->first += alt->first;
    return static_cast<mem::shared_ptr<pat>&&>(to);
  }

  bool contains(const pat_p & a, const pat_p & b) noexcept {
    for (auto & a : a->alternates) {
      if (a == b || *a == *b) {
        return true;
      }
    }
    return false;
  }

  pat_p filter_alternate(const grammar & g, pat_p && alt) noexcept {
    if (alt->type != pat_type::alternation) {
      return static_cast<pat_p &&>(alt);
    }
    pat_p result;
    pat_p pats[256];
    for (auto & a : alt->alternates) {
      if (a->type == pat_type::nonterminal) {
        result = add_alternate(static_cast<pat_p &&>(result), a);
        continue;
      }
      auto set = a->first;
      for (auto f : set) {
        if (!pats[f.index]) {
          pats[f.index] = copy(a);
          result = add_alternate(static_cast<pat_p &&>(result), pats[f.index]);
        } else if (*pats[f.index] == *a) {
          pats[f.index]->paths.add(a->paths);
        } else if (a->next) {
          pats[f.index]->next = add_alternate(static_cast<pat_p &&>(pats[f.index]->next), copy(a->next));
        } else {
          if (pats[f.index]->type == pat_type::reduce && a->type == pat_type::reduce
            && pats[f.index]->precedence != a->precedence) {
            if (pats[f.index]->precedence > a->precedence) {
              pats[f.index] = copy(a);
            }
            continue;
          }
          result = add_alternate(static_cast<pat_p &&>(result), a);
        }
      }
    }
    if (result->next) {
      result->next = filter_alternate(g, static_cast<pat_p &&>(result->next));
    }
    for (auto & a : result->alternates) {
      if (a->next) {
        a->next = filter_alternate(g, static_cast<pat_p &&>(a->next));
      }
    }
    return static_cast<pat_p&&>(result);
  }

  pat::pat(pat_alternation_t, pat_p && alt, pat_precedence precedence) noexcept
    : type(pat_type::alternation),
      precedence(precedence),
      alternates(2),
      first(alt->first)
  {
    J_ASSERT(alt && alt->type != pat_type::alternation);
    alternates.emplace_back(static_cast<pat_p &&>(alt));
  }

  pat::pat(pat_alternation_t, pat_vec && alts, pat_precedence precedence) noexcept
    : type(pat_type::alternation),
      precedence(precedence),
      alternates(static_cast<pat_vec &&>(alts))
  {
    for (auto & alt : alternates) {
      J_ASSERT(alt && alt->type != pat_type::alternation);
      first += alt->first;
    }
  }

  [[nodiscard]] bool pat::operator==(const pat & rhs) const noexcept {
    if (type != rhs.type) {
      return false;
    }
    switch (type) {
    case pat_type::reduce:
      if (reduce != rhs.reduce) {
        return false;
      }
      break;
    case pat_type::terminal:
      if (first != rhs.first) {
        return false;
      }
      break;
    case pat_type::nonterminal:
      if (nonterm != rhs.nonterm) {
        return false;
      }
      break;
    case pat_type::alternation: {
      if (alternates.size() != rhs.alternates.size()) {
        return false;
      }
      for (auto rhs_alt = rhs.alternates.begin(), alt = alternates.begin(), end = alternates.end();
           alt != end; ++alt, ++rhs_alt) {
        if (**alt != **rhs_alt) {
          return false;
        }
      }
      break;
    }
    }
    if (next == rhs.next) {
      return true;
    }
    if (!next != !rhs.next) {
      return false;
    }
    return *next == *rhs.next;
  }

  [[nodiscard]] pair<i32_t> pat::extent() const noexcept {
    pair<i32_t> extent{0,0};
    for (const pat * cur = this; cur; cur = cur->next.get()) {
      if (cur->type == pat_type::reduce) {
        extent = cur->reduce->extent();
      } else if (cur->type == pat_type::alternation) {
        for (auto & a : cur->alternates) {
          pair<i32_t> e = a->extent();
          extent.first = j::max(extent.first, e.first);
          extent.second = j::min(extent.second, e.second);
        }
      }
    }
    return extent;
  }

  [[nodiscard]] pair<i32_t> pat::length_cur_level(i32_t level) const noexcept {
    pair<i32_t> result{0, 0};
    switch (type) {
    case pat_type::terminal:
      if (first.front() == exit) {
        --level;
        J_ASSERT(level >= 0);
      } else {
        ++level;
      }
      [[fallthrough]];
    case pat_type::nonterminal:
      if (next) {
        result = next->length_cur_level(level);
      }
      if (!level) {
        ++result.first;
        ++result.second;
      }
      break;
    case pat_type::reduce: {
      break;
    }
    case pat_type::alternation:
      result.first = I32_MAX;
      for (auto & a : alternates) {
        pair<i32_t> ap = a->length_cur_level(level);
        result.first = j::min(result.first, ap.first);
        result.second = j::max(result.second, ap.second);
      }
      break;
    }
    return result;
  }

  pat_p copy(const pat_p & from) noexcept {
    pat_p result;
    if (from) {
      from->temp = result = mem::make_shared<pat>(*from);
      if (from->type == pat_type::alternation) {
        for (auto & p : result->alternates) {
          p = copy(p);
        }
      }
      result->next = copy(result->next);
    }
    return static_cast<pat_p &&>(result);
  }

  u32_t compute_hash(const pat_p & r) noexcept {
    u32_t result = 7U;
    if (r->next) {
      result = compute_hash(r->next);
    }
    switch (r->type) {
    case pat_type::nonterminal:
      return r->hash = crc32(result, r->nonterm->index);
    case pat_type::alternation:
      for (auto & a : r->alternates) {
        result = crc32(result, compute_hash(a));
      }
      return r->hash = result;
    case pat_type::reduce:
      return r->hash = crc32(9U, reductions_hash{}(r->reduce));
    case pat_type::terminal:
      return r->hash = crc32(result, terminal_set_hash{}(r->first));
    }
  }

  void pat_branch::set_paths(const pat_p & p) noexcept {
    J_ASSERT(tail);
    if (depth) {
      const auto & prev = previous();
      if (prev.type == pat_type::nonterminal) {
        p->paths = pat_paths::next(prev.paths, prev.nonterm);
      } else if (prev.type == pat_type::terminal) {
        p->paths = pat_paths::next(prev.paths, prev.first.front());
      }
    }
  }

  void pat_branch::set_reduction(red_p && red, pat_precedence prec) noexcept {
    J_ASSERT(red);
    J_ASSERT(!depth || previous().type != pat_type::reduce);
    *tail = mem::make_shared<pat>(pat_reduce_v, static_cast<red_p &&>(red), prec);
    set_paths(*tail);
  }

  void pat_branch::set_capture(i8_t index, pat_capture c, bool overwrite) noexcept {
    c.stack_offset = depth;
    caps.set(index, c, overwrite);
  }

  pat & pat_branch::previous() const noexcept {
    J_ASSERT(tail && depth);
    return *J_CONTAINER_OF(tail, pat, next);
  }

  void pat_builder::visit(const pat_term & pat) {
    J_ASSERT(!root);
    visit({0, 0, 0, 0, 0, {}}, pat, &root);
  }
  void pat_builder::visit(const pat_terms_t & pats) {
    J_ASSERT(!root);
    visit({0, 0, 0, 0, 0, {}}, pats, &root);
  }

  J_A(RNN) pat_p * add_to(pat_p * J_NOT_NULL to, pat_p && from) noexcept {
    J_ASSERT_NOT_NULL(from);
    J_ASSERT(from->type != pat_type::alternation && from->type != pat_type::reduce);
    if (!*to) {
      *to = static_cast<pat_p &&>(from);
    } else {
      J_ASSERT(from != *to);
      *to = add_alternate(static_cast<pat_p &&>(*to), static_cast<pat_p &&>(from));
    }
    return &from->next;
  }

  void pat_builder::visit(pat_builder_ctx ctx, const pat_terms_t & pat, pat_p * J_NOT_NULL to) {
    if (!pat) {
      branches.push_back({ to, ctx.depth, ctx.reduction_depth, (i8_t)ctx.num_root_elements, (i8_t)(ctx.reduction_tree_depth_zero - ctx.reduction_tree_depth), ctx.caps });
      return;
    }
    for (auto & p : pat) {
      visit(ctx, *p, to);
    }
  }

  void pat_builder::visit(pat_builder_ctx ctx, const pat_term & p, pat_p * J_NOT_NULL to) {
    if (ctx.lookahead_depth) {
      J_ASSERT(p.type == pat_term::type_any);
      if (p.capture_index >= 0) {
        ctx.caps.set(p.capture_index - 1, {arg_type::select_term_ahead, (i8_t)-ctx.lookahead_depth});
      }
      ctx.lookahead_depth++;
    } else {
      switch (p.type) {
      case pat_term::type_anchor:
        J_ASSERT(!ctx.lookahead_depth);
        ctx.reduction_depth = 0;
        ctx.reduction_tree_depth_zero = ctx.reduction_tree_depth;
        break;
      case pat_term::type_end:
        J_ASSERT(!ctx.lookahead_depth);
        ctx.lookahead_depth = 1;
        break;
      case pat_term::type_any:
        to = add_to(to, mem::make_shared<pat>(pat_nonterminal_v, this->any, prec(pat_prec_type::shift_any)));
        ctx.add_element(1);
        if (p.capture_index >= 0) {
          ctx.caps.set(p.capture_index - 1, {arg_type::capture_term, (i8_t)ctx.depth});
        }
        break;
      case pat_term::type_term: {
        i32_t n = 2;
        to = add_to(to, mem::make_shared<pat>(pat_terminal_v, terminal{p.name.term->index}, prec(pat_prec_type::shift)));
        for (auto in : p.name.term->operands()) {
          if (in->is_input()) {
            ++n;
            to = add_to(to, mem::make_shared<pat>(pat_nonterminal_v, in->has_dynamic_size ? anys : any, prec(in->has_dynamic_size ? pat_prec_type::shift_anys : pat_prec_type::shift_any)));
          }
        }
        to = add_to(to, mem::make_shared<pat>(pat_terminal_v, exit, prec(pat_prec_type::shift)));
        ctx.add_element(n);
        if (p.capture_index >= 0) {
          ctx.caps.set(p.capture_index - 1, {arg_type::capture_term, (i8_t)ctx.depth});
        }
        break;
      }
      case pat_term::type_ctor:
        to = add_to(to, mem::make_shared<pat>(pat_terminal_v, terminal{p.name.term->index}, prec(pat_prec_type::shift)));
        ++ctx.reduction_tree_depth;
        ctx.add_depth(1);
        if (p.capture_index >= 0) {
          ctx.caps.set(p.capture_index - 1, {arg_type::capture_term, (i8_t)ctx.depth});
        }
        break;
      case pat_term::type_exit:
        --ctx.reduction_tree_depth;
        to = add_to(to, mem::make_shared<pat>(pat_terminal_v, exit, prec(pat_prec_type::shift)));
        ctx.add_element(1);
        if (p.capture_index >= 0) {
          ctx.caps.set(p.capture_index - 1, {arg_type::capture_term, (i8_t)ctx.depth});
        }
        break;
      case pat_term::type_nt: {
        nt_p nt = p.name.nt;
        J_ASSERT(nt && nt->index);
        to = add_to(to, mem::make_shared<pat>(pat_nonterminal_v, nt, prec(pat_prec_type::shift)));
        ctx.add_element(1);
        if (p.capture_index >= 0) {
          ctx.caps.set(p.capture_index - 1, {p.capture_index == 0 ? arg_type::capture_this_reduction_nt : arg_type::capture_reduction_copy_nt, (i8_t)ctx.depth, nt}, true);
        }
        for (i32_t i = 0, max = p.name.nt->num_fields(); i != max; ++i) {
          if (p.nt_captures[i]) {
            auto & field = nt->fields.at(i);
            ctx.caps.set(p.nt_captures[i] - 1, {arg_type::capture_reduction_field, (i8_t)ctx.depth, field.type, nt, (u8_t)field.offset});
          }
        }
        break;
      }
      case pat_term::type_none:
        J_FAIL("Unexpected none");
      }
    }
    if (!p.next) {
      branches.push_back({ to, ctx.depth, ctx.reduction_depth, (i8_t)ctx.num_root_elements, (i8_t)(ctx.reduction_tree_depth_zero - ctx.reduction_tree_depth), ctx.caps });
    } else {
      visit(ctx, p.next, to);
    }
  }

  pat_builder::pat_builder(const module & mod, pat_layer layer, i8_t precedence, i32_t index) noexcept
    : mod(mod),
      layer(layer),
      precedence(precedence),
      index(index),
      any((nt_p)mod.node_at("Any", node_nonterminal)),
      anys((nt_p)mod.node_at("Anys", node_nonterminal))
  { }

  [[nodiscard]] pat_tree * pat_builder::build() noexcept {
    bool is_single_ctor = true, is_single_nonterminal = true, has_lookbehind = false, has_depth_difference = false;
    for (auto & tail : branches) {
      if (tail.num_root_elements != 1) {
        is_single_ctor = false;
        is_single_nonterminal = false;
      }
      has_lookbehind |= tail.depth != tail.reduction_depth;
      has_depth_difference |= tail.depth_difference;
      auto & prev = tail.previous();
      if (prev.type != pat_type::terminal) {
        is_single_ctor = false;
      }
      if (prev.type != pat_type::nonterminal) {
        is_single_nonterminal = false;
      }
      if (tail.caps.this_capture) {
        tail.caps.this_capture.stack_offset = tail.depth - tail.caps.this_capture.stack_offset;
      }
      for (auto & cap : tail.caps) {
        if (cap && cap.stack_offset >= 0) {
          cap.stack_offset = tail.depth - cap.stack_offset;
        }
      }
    }
    pat_tree * tree = ::new pat_tree{
      static_cast<pat_p &&>(root),
      static_cast<noncopyable_vector<pat_branch> &&>(branches),

      is_single_ctor,
      is_single_nonterminal,
      has_lookbehind,
      has_depth_difference,
    };
    if (!tree->branches) {
      tree->branches.push_back({ &tree->root, 0, 0, 0, 0, {}});
    }
    return tree;
  }

  J_A(HIDDEN) pat_precedence pat_builder::prec(pat_prec_type t) const noexcept {
    return pat_precedence{layer, precedence, t, index};
  }

  [[nodiscard]] bool pat_elem::operator<(const pat_elem & rhs) const noexcept {
    if (!nt != !rhs.nt) {
      return rhs.nt;
    }
    if (nt) {
      return nt->index < rhs.nt->index;
    }
    return t.index < rhs.t.index;
  }

  [[nodiscard]] bool overlaps(const pat_elem_vec & lhs, const pat_elem_vec & rhs) noexcept {
    if (lhs.size() == rhs.size()) {
      return false;
    }
    for (auto it = lhs.end() - 1, it2 = rhs.end() - 1, end = it - j::min(lhs.size(), rhs.size()); it != end; --it, --it2) {
      if (*it != *it2) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] u32_t pat_elem_vec_hash::operator()(const pat_elem_vec & v) const noexcept {
    u32_t result = 121;
    for (auto & p : v) {
      result = crc32(result, p.nt ? -p.nt->index : p.t.index);
    }
    return result;
  }

  [[nodiscard]] bool pat_elem_vec_hash::operator()(const pat_elem_vec & lhs, const pat_elem_vec & rhs) const noexcept {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    const pat_elem * pat2 = rhs.begin();
    for (const pat_elem & pat1 : lhs) {
      if (pat1 != *pat2) {
        return false;
      }
      ++pat2;
    }
    return true;
  }

  [[nodiscard]] pat_paths pat_paths::next(const pat_paths & prev, pat_elem e) noexcept {
    pat_paths result;
    if (prev.vecs) {
      for (pat_elem_vec v : prev.vecs) {
        v.push_back(e);
        result.add(static_cast<pat_elem_vec &&>(v));
      }
    } else {
      pat_elem_vec vec;
      vec.push_back(e);
      result.add(static_cast<pat_elem_vec &&>(vec));
    }
    return result;
  }

  bool pat_paths::add(pat_elem_vec && rhs) noexcept {
    u32_t sz = rhs.size();
    if (!sz || vecs.contains(rhs)) {
      return false;
    }
    for (auto it = vecs.begin(), end = vecs.end(); it != end;) {
      if (overlaps(*it, rhs)) {
        if (it->size() < sz) {
          it = vecs.erase(it);
          continue;
        }
        return false;
      }
      ++it;
    }
    vecs.emplace(static_cast<pat_elem_vec &&>(rhs));
    return true;
  }

  bool pat_paths::add(const pat_elem_vec & rhs) noexcept {
    u32_t sz = rhs.size();
    if (!sz || vecs.contains(rhs)) {
      return false;
    }
    for (auto it = vecs.begin(), end = vecs.end(); it != end;) {
      if (overlaps(*it, rhs)) {
        if (it->size() < sz) {
          it = vecs.erase(it);
          continue;
        }
        return false;
      }
      ++it;
    }
    vecs.emplace(rhs);
    return true;
  }

  bool pat_paths::add(const pat_paths & rhs) noexcept {
    bool did_add = false;
    if (J_LIKELY(&rhs != this)) {
      for (auto & v : rhs.vecs) {
        did_add |= add(v);
      }
    }
    return did_add;
  }

  [[nodiscard]] vector<pat_elem_vec> pat_paths::items() const noexcept {
    vector<pat_elem_vec> result(vecs.size());
    for (auto & v : vecs) {
      result.push_back(v);
    }
    algo::quicksort(result.begin(), result.end());
    return result;
  }

  [[nodiscard]] bool operator<(const pat_elem_vec & lhs, const pat_elem_vec & rhs) noexcept {
    const pat_elem *l = lhs.begin(), *le = lhs.end(), *r = rhs.begin(), *re = rhs.end();
    for (; l != le && r != re; ++l, ++r) {
      if (*l != *r) {
        return *l < *r;
      }
    }
    return r != re;
  }
  J_A(RNN,NODISC) pat_tree * make_empty_pat_tree() noexcept {
    pat_tree * result = ::new pat_tree{pat_p{}, {}, false, false, false, false};
    result->branches.push_back({ &result->root, 0U, 0U, 0, 0, {} });
    return result;
  }

  [[nodiscard]] pat_term_p pat_term::clone() const noexcept {
    pat_term_p result = j::mem::make_shared<pat_term>(*this);
    for (auto & out : result->next) {
      J_ASSERT(out);
      out = out->clone();
    }
    return result;
  }

  void pat_term::clear() noexcept {
    type = type_none;
    name.nt = nullptr;
    capture_index = -1;
  }
}
