#pragma once

#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/passes/trs_write_expr.hpp"
#include "lisp/env/context.hpp"
#include "lisp/air/passes/air_pass.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/passes/const_fold.hpp"
#include "meta/attr_value.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/air/exprs/write_expr.hpp"
#include "strings/format.hpp"
#include "exceptions/assert.hpp"
#include "logging/global.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/air/passes/term_stack.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    using exprs::expr;
    namespace s = strings;

    using format_fn_t J_NO_DEBUG_TYPE = void(*)(const void * J_NOT_NULL data) noexcept;

    struct J_TYPE_HIDDEN nt_info final {
      const char * name = nullptr;
      format_fn_t format = nullptr;
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-internal"
    /// Index of the exit token (comes after terminal tokens.)
    J_A(ND) extern const i32_t exit_token;
    /// Set when the parser is build with `--debug-print` option.
    J_A(ND) extern const bool debug_print_enabled;
    /// Names of all terminals, including eof and exit.
    J_A(ND) extern const i32_t * const gotos[];
    J_A(ND) extern const i32_t any_gotos[];
    J_A(ND) extern const i32_t anys_gotos[];
    J_A(ND) extern const nt_info nt_infos[];
#pragma clang diagnostic pop

    struct J_TYPE_HIDDEN stack_entry final {
      expr * e = nullptr;
      i32_t state = 0;
      i32_t type = 0;
      char * data = nullptr;
    };

    struct J_TYPE_HIDDEN expr_it final {
      expr * e;
      i32_t index;

      explicit expr_it(expr * J_NOT_NULL e) noexcept
        : e(e),
          index(-1)
      { }

      [[nodiscard]] expr * lookahead_expr() const noexcept {
        return index == -1 ? e : (e && index < e->num_inputs ? e->in_expr(index) : nullptr);
      }

      [[nodiscard]] inline i32_t lookahead() const noexcept {
        if (J_UNLIKELY(!e)) {
          return exit_token;
        } else if (J_UNLIKELY(index == -1)) {
          return (i32_t)e->type;
        }
        const i32_t num_inputs = e->num_inputs;
        if (index >= num_inputs) {
          return exit_token;
        }
        return (i32_t)e->in_expr(index)->type;
      }

      J_A(AI) inline bool next() noexcept {
        const i32_t num_inputs = e->num_inputs;
        if (index >= num_inputs) {
          index = e->parent_pos + 1;
          e = e->parent;
          return false;
        }
        if (J_LIKELY(index >= 0)) {
          e = e->in_expr(index);
        }
        index = 0;
        return true;
      }
    };

    [[maybe_unused]] J_A(NI) inline void format_stack_alloc(term_stack * stk, expr *** marker, i32_t num = 1) noexcept {
      if (!marker || !marker[0]) {
        J_DEBUG_INL("{#gray}∅{/}");
        return;
      }
      i32_t start = marker[-num] - stk->data_begin;
      i32_t end = marker[0] - stk->data_begin;
      if (start == end) {
        J_DEBUG_INL("{#light_gray}@{}{/}", start);
        return;
      }

      J_DEBUG_INL("{#bright_yellow,bold}{}{/}{#yellow}-{}{/}", start, end);
  }

  J_A(NI) inline void format_expr(const expr * e, bool dim = false) noexcept {
    if (!e) {
      J_DEBUG_INL("{#bright_red}None{/}");
      return;
    }
    switch(e->type) {
    case expr_type::ld_const:
      J_DEBUG_INL(dim ? "{#green}{}{/}" : "{#green,bold}{}{/}", *(const lisp_imm*)(e + 1));
      break;
    case expr_type::sym_val_rd:
    case expr_type::lex_rd:
    case expr_type::lex_rd_bound:
    case expr_type::sym_val_wr:
    case expr_type::lex_wr:
    case expr_type::lex_wr_bound:
      // return s::format("{#bright_red}{}{/}", *(const id*)(e->static_begin()));
    default:
      J_DEBUG_INL(dim ? "{#light_gray}{:-}{/}" : "{}", e->type);
      break;
    }
  }

  J_A(NI) inline void format_term_stack(const term_stack & stack, term_stack_type type) noexcept {
    if (stack.markers_begin == stack.markers) {
      return;
    }
    i32_t interval;
    switch (type) {
    case stk_collect:
      interval = 1;
      J_DEBUG_INL("    {#bright_cyan}Collect:{/}"); break;
    case stk_side_fx:
      interval = 2;
      J_DEBUG_INL("    {#bright_yellow}Side FX:{/}");
      break;
    default: J_FAIL("Unknown stack type");
    }
    bool is_first = true;
    i32_t i = 0;
    for (expr ***it = stack.begin(), ***end = stack.end(); it != end; ++it, ++i) {
      J_ASSERT(it < end);
      term_stack_allocation a{const_cast<term_stack*>(&stack), it + 1};
      if (!is_first) {
        J_DEBUG_INL((i % interval) ? "{#light_gray}⁞{/}" : "{#light_gray,bold}|{/}");
      }
      is_first = false;
      if (a) {
        bool is_first_expr = true;
        for (auto & e : a) {
          if (!is_first_expr) {
            J_DEBUG_INL(" ");
          }
          is_first_expr = false;
          format_expr(e);
        }
      } else if (interval == 1){
        J_DEBUG_INL("{#gray}·{/}");
      }
    }
    J_DEBUG("");
  }

  [[maybe_unused]] J_A(AI) inline void replace(expr * J_NOT_NULL & e, expr * J_NOT_NULL new_expr) noexcept {
    J_ASSERT_NOT_NULL(e->parent);
    e = e->parent->set_input(e->parent_pos, new_expr);
  }

  struct J_TYPE_HIDDEN parser_state final {
    stack_entry * stack = nullptr;
    char * data_stack = nullptr;
    stack_entry ** expr_stack = nullptr;
    expr_it iter;
    term_stack collect_stack;
    term_stack side_fx_stack;
    i32_t lookahead;
    bool done = false;
    i32_t num_shifts = 0;
    i32_t num_reductions = 0;
    j::mem::bump_pool & pool;
    const const_fold_table & fold_table;

    stack_entry * stack_begin = nullptr;
    stack_entry ** expr_stack_begin = nullptr;

    exprs::expr * root = nullptr;

    // expr * J_NOT_NULL e, j::mem::bump_pool & pool
    parser_state(j::mem::bump_pool & pool, expr * J_NOT_NULL root) noexcept
      : stack((stack_entry*)pool.allocate_aligned_zero(sizeof(stack_entry) * 128U, 16U)),
        data_stack((char*)pool.allocate_aligned_zero(2048U, 8U)),
        expr_stack((stack_entry**)pool.allocate_aligned_zero(64U * 8U, 8U)),
        iter(root),
        collect_stack(pool, 256, 32),
        side_fx_stack(pool, 256, 32),
        lookahead(iter.lookahead()),
        pool(pool),
        fold_table(*get_const_fold_table(env::env_context->env)),
        stack_begin(stack),
        expr_stack_begin(expr_stack),
        root(root)
    {
      *stack = stack_entry{this->root, 0, 0, this->data_stack};
      J_ASSERT(root->type == expr_type::fn_body);
      if (debug_print_enabled) {
        J_DEBUG("Root: {}", *this->root);
      }
    }

    void check_empty() {
      J_ASSERT(side_fx_stack.markers = side_fx_stack.markers_begin);
      J_ASSERT(collect_stack.markers = collect_stack.markers_begin);
    }

    template<typename T>
    J_A(RNN,ND,ALIGN(8)) inline T * nt(i32_t stack_offset) const noexcept {
      J_ASSERT(stack_offset >= 0 && stack - stack_offset >= stack_begin);
      return reinterpret_cast<T *>(stack[-stack_offset].data);
    }

    template<typename T>
    [[nodiscard]] inline T & field(i32_t stack_offset, i32_t field_offset) noexcept {
      J_ASSERT(stack_offset >= 0 && stack - stack_offset >= stack_begin && field_offset >= 0);
      return *reinterpret_cast<T *>(stack[-stack_offset].data + field_offset);
    }

    void dump_stack_entry(const stack_entry * J_NOT_NULL entry) const noexcept {
      bool did_print = false;
      if (entry->type) {
        if (entry->type == -1) {
          J_DEBUG_INL("{#red,bold}Exit{/}");
          return;
        }
        const nt_info & ni = nt_infos[entry->type];
        did_print = true;
        J_DEBUG_INL("{#bright_cyan,bold}{}{/}", ni.name);
        if (ni.format) {
          J_DEBUG_INL("{#bold}({/}");
          ni.format(entry->data);
          J_DEBUG_INL("{#bold}){/}");
          return;
        }
      }
      if (entry->type != 1 && entry->e) {
        if (did_print) {
          J_DEBUG_INL(":");
        }
        format_expr(entry->e);
      }
    }

    template<typename... Args>
    void dump_stacks(const char * J_NOT_NULL message = "", const Args & ... args) const noexcept {
      J_LOG_RESET_COL();
      const stack_entry * const * expr_stack_next = expr_stack_begin;
      J_DEBUG_INL("{#bright_cyan,bold}#{:+2}{/} {#bright_yellow,bold}S#{:+3}{/}", num_shifts + num_reductions, stack->state);
      for (const stack_entry * entry = stack_begin + 2; entry <= stack; ++entry) {
        J_DEBUG_INL(" "); if (*expr_stack_next == entry) {
          J_DEBUG_INL("{#bright_yellow}*{/}");
          ++expr_stack_next;
        }
        dump_stack_entry(entry);
      }
      J_DEBUG_INL("{#bright_yellow,bold}|{/}");
      if (lookahead == exit_token) {
        J_DEBUG_INL("{#red}Exit{/}");
      } else {
        format_expr(iter.lookahead_expr(), true);
      }
      if (*message) {
        J_LOG_PAD_TO_COL(64);
      }
      J_DEBUG(message, args...);
      format_term_stack(collect_stack, stk_collect);
      format_term_stack(side_fx_stack, stk_side_fx);
    }

    J_A(NORET,NI,COLD) void fail(const char * error = nullptr) const {
      dump_stacks(error ? error : "{#bright_redi,bold}Parse error{/}");
      J_FAIL("Parse error, {}", error);
    }

      void shift(i32_t goto_index) noexcept {
        ++num_shifts;
        expr * e = iter.e;
        if (iter.next()) {
          e = iter.e;
          *expr_stack++ = stack;
        } else {
          --expr_stack;
        }
        *++stack = { e, goto_index, lookahead == exit_token ? -1 : 0, data_stack};
        lookahead = iter.lookahead();
      }

      J_A(AI,NODISC,ND) inline term_stack & get_term_stack(term_stack_type type) noexcept {
        switch (type) {
        case stk_collect: return collect_stack;
        case stk_side_fx: return side_fx_stack;
        default: J_FAIL("Unknown stack type");
        }
      }

      J_A(AI) inline void update_lookahead() noexcept {
        lookahead = iter.lookahead();
      }

      inline void copy_reduce(i32_t reduce_depth, i32_t stack_offset, u32_t sz) noexcept {
        const void * t J_A(ALIGN_VAR(8)) = stack[-stack_offset].data;
        J_ASSERT(sz == (stack_offset == 0 ? data_stack : (char*)stack[1-stack_offset].data) - (char*)t);
        reduce(reduce_depth, stack[-stack_offset].type);
        j::memmove(J_ASSUME_ALIGNED_8(data_stack), t, sz);
        data_stack += sz;
      }

      inline void copy_reduce(i32_t reduce_depth, i32_t stack_offset) noexcept {
        const void * t J_A(ALIGN_VAR(8)) = stack[-stack_offset].data;
        i32_t sz = (stack_offset == 0 ? data_stack : (char*)stack[1-stack_offset].data) - (char*)t;
        reduce(reduce_depth, stack[-stack_offset].type);
        j::memmove(J_ASSUME_ALIGNED_8(data_stack), t, sz);
        data_stack += sz;
      }

      inline void copy_reduce(i32_t reduce_depth, i32_t stack_offset, u32_t sz, i32_t type) noexcept {
        const void * t J_A(ALIGN_VAR(8)) = stack[-stack_offset].data;
        J_ASSERT(sz == (stack_offset == 0 ? data_stack : (char*)stack[1-stack_offset].data) - (char*)t);
        reduce(reduce_depth, type);
        j::memmove(J_ASSUME_ALIGNED_8(data_stack), t, sz);
        data_stack += sz;
      }

      template<typename T, typename... Ts>
      J_A(AI,HIDDEN) inline void reduce(i32_t reduce_depth, i32_t type, Ts... data) noexcept {
        reduce(reduce_depth, type);
        ::new (J_ASSUME_ALIGNED_8(data_stack)) T{static_cast<Ts &&>(data)...};
        data_stack += sizeof(T);
      }

      void reduce(i32_t reduce_depth, i32_t type) noexcept {
        ++num_reductions;
        J_ASSERT(stack - stack_begin >= reduce_depth && type >= 0);
        stack = reduce_depth >= 0 ? stack - reduce_depth : *expr_stack;
        if (stack == stack_begin) {
          done = true;
          return;
        }
        i32_t state = stack->state;
        i32_t to_state = type == 0 ? any_gotos[state] : type == 1 ? anys_gotos[state] : gotos[state][type - 2];
        ++stack;
        J_ASSERT(stack >= stack_begin);
        stack->state = to_state;
        stack->type = type;
        if (reduce_depth) {
          data_stack = stack->data;
        } else {
          stack->data = data_stack;
          stack->e = nullptr;
        }
      }

      void pop(i32_t num) noexcept {
        ++num_reductions;
        J_ASSERT(stack - stack_begin > num);
        stack -= num;
        data_stack = stack[1].data;
        // collect_stack = stack[1].collect_stack;
      }

      void rewind(i32_t num, i32_t depth_difference) noexcept {
        ++num_reductions;
        J_ASSERT(stack - stack_begin > num);
        stack -= num;
        expr_stack += depth_difference;
        expr * target = stack[1].e, *parent = target->parent;
        J_ASSUME_NOT_NULL(target, parent);
        iter.index = target->parent_pos;
        iter.e = parent;
        // J_DEBUG("Erasing {} in {}:{}", target->type, parent->type, pos);
        // J_DEBUG("State is {}:{}", iter.e->type, iter.index);
        data_stack = stack->data;
        // collect_stack = stack->collect_stack;
        update_lookahead();
      }

      void erase() noexcept {
        ++num_reductions;
        J_ASSERT(stack - stack_begin > 1U);
        expr * target = stack->e, *parent = target->parent;
        u32_t pos = target->parent_pos;
        if (debug_print_enabled) {
          J_DEBUG("Erasing {} in {}:{}", target->type, parent->type, pos);
        }
        // J_DEBUG("State is {}:{}", iter.e->type, iter.index);
        --iter.index;
        J_ASSUME_NOT_NULL(target, parent);
        --parent->num_inputs;
        exprs::input * it = parent->inputs_begin() + pos;
        ::j::memmove(parent->inputs_begin() + pos, it, (parent->num_inputs - pos)* sizeof(exprs::input));
        for (exprs::input * e = parent->inputs_end();it != e; ++it) {
          e->expr->parent_pos--;
        }
        --stack;
        data_stack = stack->data;
        // collect_stack = stack->collect_stack;
      }

      void reduce_1(i32_t type) noexcept {
        ++num_reductions;
        J_ASSERT(stack > stack_begin && type >= 0);
        i32_t state = stack[-1].state;
        stack->state = type == 0 ? any_gotos[state] : type == 1 ? anys_gotos[state] : gotos[state][type - 2];
        stack->type = type;
        data_stack = stack->data;
      }

      void reduce_any(i32_t reduce_depth) noexcept {
        ++num_reductions;
        stack = reduce_depth >= 0 ? stack - reduce_depth : *expr_stack;
        if (stack == stack_begin) {
          done = true;
          return;
        }
        ++stack;
        stack->state = any_gotos[stack[-1].state];

        stack->type = 0;
        data_stack = stack->data;
      }

      J_A(NODISC,RNN) expr * mk_const(lisp_imm imm) const noexcept {
        return trs_construct(expr_type::ld_const, pool, 0, imm);
      }

      J_A(NODISC,RNN) expr * mk_const(lisp_imm imm, expr * old_expr) const noexcept {
        if (old_expr && old_expr->type == expr_type::ld_const) {
          J_ASSERT(old_expr->get_const().eq(imm));
          return old_expr;
        }
        return mk_const(imm);
      }

      template<typename T>
      J_A(NODISC,RNN) expr * wrap_many(expr_type type, const T & values) const noexcept {
        return values.size() == 1 ? values[0] : trs_wr(type, pool, values);
      }

      J_A(NODISC,RNN) expr * maybe_ineg(i8_t mul, expr * J_NOT_NULL e) const noexcept {
        return mul == 1 ? e : trs_wr(expr_type::ineg, pool, e);
      }

      J_A(NODISC,RNN) expr * wrap_many_move(expr_type type, term_stack_allocation & to, term_stack_allocation & from) const noexcept {
        expr * r = to.size() == 1 ? to[0] : trs_wr(type, pool, to);
        span<expr*> s = to;
        to += from;
        to.stack->erase(s);
        return r;
      }

      J_A(NODISC,RNN) expr * wrap_one_many(expr_type one_type, expr_type many_type, span<expr*> values) const noexcept {
        return trs_wr(values.size() == 1 ? one_type : many_type, pool, values);
      }

      J_A(NODISC,RNN) expr * wrap_progx(split_tsa & fx) noexcept {
        if (!fx) {
          return mk_const(lisp_nil_v);
        }
        if (fx.size() == 1) {
          return fx[0];
        }
        auto pre = fx.pre_fx(), post = fx.post_fx();
        J_ASSERT(pre);
        expr * e = pre.size() == 1 ? pre[0] : trs_wr(expr_type::progn, pool, pre);
        if (post) {
          e = trs_wr(expr_type::prog1, pool, e, post);
        }
        fx.release();
        return e;
      }

      J_A(NODISC,RNN) expr * wrap_fx_impl(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        if (fx) {
          auto pre = fx.pre_fx(), post = fx.post_fx();
          if (pre) {
            e = trs_wr(expr_type::progn, pool, pre, e);
          }
          if (post) {
            e = trs_wr(expr_type::prog1, pool, e, post);
          }
        }
        return e;
      }

      J_A(RNN) expr * wrap_fx(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        e = wrap_fx_impl(e, fx);
        fx.release();
        return e;
      }

      J_A(RNN) expr * wrap_fx(expr * J_NOT_NULL e, split_tsa && fx) noexcept {
        e = wrap_fx_impl(e, fx);
        fx.release();
        return e;
      }

      J_A(NODISC) split_span join(split_tsa & pre, split_tsa & post) noexcept {
        J_ASSERT(pre.marker[0] == post.marker[-2]);
        split_span result{pre.marker[-1], pre.marker[0], post.marker[-1]};
        return result;
      }

      span<expr*> wrap_fx(span<expr*> span, split_tsa & pre, split_tsa & post) noexcept {
        if (!span) {
          pre.to_pre_fx();
          post.to_post_fx();
          return span;
        }
        auto parts = join(pre, post);
        if (parts.num_pre_fx()) {
          span.front() = trs_wr(expr_type::progn, pool, parts.pre_fx(), span.front());
        }
        if (parts.num_post_fx()) {
          span.back() = trs_wr(expr_type::prog1, pool, span.back(), parts.post_fx());
        }
        return span;
      }

      J_A(RNN,NODISC) expr * wrap_fx(expr * J_NOT_NULL e, split_tsa & a, split_tsa & b) noexcept {
        expr ** split = a.marker[-1];
        split_tsa* pre = a.marker > b.marker ? &b : &a;
        split_tsa* post = a.marker > b.marker ? &a : &b;
        if (pre->marker[-2] != split) {
          e = trs_wr(expr_type::progn, pool, span(pre->marker[-2], split), e);
        }
        if (post->marker[0] != split) {
          e = trs_wr(expr_type::prog1, pool, e, span(split, post->marker[0]), e);
        }
        post->release();
        pre->release();
        return e;
      }

      J_A(RNN,NODISC) expr * apply_pre_fx(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        if (auto mid = span(fx.marker[-2], fx.marker[-1])) {
          e = trs_wr(expr_type::progn, pool, mid, e);
          fx.stack->erase(mid);
        }
        return e;
      }
      J_A(RNN,NODISC) expr * apply_post_fx(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        if (auto mid = span(fx.marker[-1], fx.marker[0])) {
          e = trs_wr(expr_type::prog1, pool, e, mid);
          fx.marker[0] = fx.marker[-1];
        }
        return e;
      }
      J_A(RNN,NODISC) expr * apply_pre_fx_as_post_fx(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        if (auto mid = span(fx.marker[-2], fx.marker[-1])) {
          e = trs_wr(expr_type::prog1, pool, e, mid);
          fx.stack->erase(mid);
        }
        return e;
      }
      J_A(RNN,NODISC) expr * apply_post_fx_as_pre_fx(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        if (auto mid = span(fx.marker[-1], fx.marker[0])) {
          e = trs_wr(expr_type::progn, pool, mid, e);
          fx.marker[0] = fx.marker[-1];
        }
        return e;
      }

      J_A(RNN,NODISC) expr * apply_mid_pre_fx(expr * J_NOT_NULL e, split_tsa & a, split_tsa & b) noexcept {
        if (auto mid = span((a.marker > b.marker ? b : a).marker[-1], (a.marker > b.marker ? a : b).marker[-1])) {
          e = trs_wr(expr_type::progn, pool, mid, e);
          a.stack->erase(mid);
        }
        return e;
      }

      J_A(RNN,NODISC) expr * apply_mid_post_fx(expr * J_NOT_NULL e, split_tsa & a, split_tsa & b) noexcept {
        if (auto mid = span((a.marker > b.marker ? b : a).marker[-1], (a.marker > b.marker ? a : b).marker[-1])) {
          e = trs_wr(expr_type::prog1, pool, e, mid);
          a.stack->erase(mid);
        }
        return e;
      }

      split_tsa & merge(term_stack_allocation & lhs, term_stack_allocation & rhs, split_tsa & pre, split_tsa & post) noexcept {
        J_ASSERT(lhs);
        expr ** pre_mid = pre.marker[-1];
        expr ** post_mid = post.marker[-1];
        pre.inplace_concat_fx(post);
        if (lhs && rhs) {
          rhs.begin()[0] = trs_wr(expr_type::progn, pool, span(pre_mid, post_mid), rhs.begin()[0]);
          pre.stack->erase(span(pre_mid, post_mid));
          pre.marker[-1] = pre_mid;
        } else if (lhs) {
          pre.marker[-1] = pre_mid;
        } else {
          pre.marker[-1] = post_mid;
        }
        lhs += rhs;
        return pre;
      }

      split_tsa & merge_full(term_stack_allocation & lhs, term_stack_allocation & rhs, split_tsa & pre, split_tsa & post) noexcept {
        J_ASSERT(lhs);
        expr ** pre_mid = pre.marker[-1];
        expr ** post_mid = post.marker[-1];
        pre.inplace_concat_fx(post);
        if (*pre_mid != *post_mid) {
          if (rhs) {
            rhs.begin()[0] = trs_wr(expr_type::progn, pool, span(pre_mid, post_mid), rhs.begin()[0]);
          } else {
            J_ASSERT(lhs);
            lhs.end()[-1] = trs_wr(expr_type::progn, pool, span(pre_mid, post_mid), lhs.begin()[-1]);
          }
        }
        pre.stack->erase(span(pre_mid, post_mid));
        pre.marker[-1] = pre_mid;
        lhs += rhs;
        return pre;
      }

      J_A(RNN) expr * wrap_fx_clear(expr * J_NOT_NULL e, split_tsa & fx) noexcept {
        e = wrap_fx_impl(e, fx);
        fx.clear();
        return e;
      }

      J_A(AI) inline expr * get_ctor_arg(expr * e) const noexcept {
        return e;
      }

      J_A(AI) inline span<expr*> get_ctor_arg(span<expr*> & e) const noexcept {
        return e;
      }

      J_A(AI) inline span<expr*> get_ctor_arg(split_span & e) const noexcept {
        return e;
      }

      template<int I>
      J_A(AI) inline span<expr*> get_ctor_arg(tsa_base<I> & e) const noexcept {
        return e.release();
      }

      template<typename... Ts>
      J_A(NODISC,RNN) expr * wrap_pre_fx_no_rel(expr * J_NOT_NULL e, Ts ... fx) noexcept {
        return (fx || ...) ? trs_wr(expr_type::progn, pool, fx..., e) : e;
      }

      template<typename... Ts>
      J_A(NODISC,RNN) expr * wrap_post_fx_no_rel(expr * J_NOT_NULL e, Ts... fx) noexcept {
        return (fx || ...) ? trs_wr(expr_type::prog1, pool, e, fx...) : e;
      }

      template<typename... Ts>
      J_A(NODISC,RNN) expr * wrap_pre_fx(expr * J_NOT_NULL e, Ts ... fx) noexcept {
        return (fx || ...) ? trs_wr(expr_type::progn, pool, get_ctor_arg(fx)..., e) : ((get_ctor_arg(fx), ...), e);
      }

      template<typename... Ts>
      J_A(NODISC,RNN) expr * wrap_post_fx(expr * J_NOT_NULL e, Ts... fx) noexcept {
        return (fx || ...) ? trs_wr(expr_type::prog1, pool, e, get_ctor_arg(fx)...) : ((get_ctor_arg(fx), ...), e);
      }

    inline void replace_top(expr * J_NOT_NULL new_expr) noexcept {
        J_ASSERT_NOT_NULL(stack->e, stack->e->parent);
        stack->e = stack->e->parent->set_input(stack->e->parent_pos, new_expr);
        reduce_any(1);
      }
    };
  }
}
