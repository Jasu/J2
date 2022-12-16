#pragma once

#include "lisp/common/id.hpp"
#include "parsing/precedence_parser.hpp"
#include "meta/errors.hpp"
#include "meta/value_types.hpp"
#include "meta/rules/args.hpp"

namespace j::meta {
  struct dump_context;
}
namespace j::meta::inline rules {
  extern constinit const parsing::operator_info operator_info[num_operator_types_v];
  struct id_resolver;

  enum class term_expr_use : u8_t {
    normal,
    copy,
  };

  enum class term_expr_type : u8_t {
    none,
    arg,
    term_construct,
    constant,
    prop_read,
    member_fn_call,
    global_fn_call,
    ternary,
    binop,
  };

  enum class term_expr_type_kind : u8_t {
    none,
    attr_value_type,
    val_type,
    blob,
    term_or_terms,
    nt,
    nt_type,
    any,
  };

  struct term_expr;

  struct te_type final {
    J_A(AI,ND,HIDDEN) inline te_type() noexcept : value_type{nullptr} {}

    te_type(const te_type & rhs) noexcept;
    te_type(te_type && rhs) noexcept;

    te_type & operator=(const te_type & rhs) noexcept;
    te_type & operator=(te_type && rhs) noexcept;

    J_A(AI,ND,HIDDEN) explicit inline te_type(term_expr_type_kind kind, i32_t size = 0U) noexcept
      : kind{kind},
        size(size)
    { }

    te_type(const attr_type_def & t) noexcept;
    te_type(attr_type_def && t) noexcept;

    J_A(AI,ND,HIDDEN) inline te_type(nt_p J_NOT_NULL t) noexcept
      : kind{term_expr_type_kind::nt},
        nt(t)
    { }

    J_A(AI,ND,HIDDEN) inline te_type(nt_data_type * J_NOT_NULL t) noexcept
      : kind{term_expr_type_kind::nt_type},
        nt_type(t)
    { }

    J_A(AI,ND,HIDDEN) inline te_type(val_type * J_NOT_NULL t) noexcept
      : kind{term_expr_type_kind::val_type},
        value_type(t)
    { }

    void clear() noexcept;

    [[nodiscard]] bool operator==(const te_type & rhs) const noexcept;

    [[nodiscard]] inline bool operator==(const val_type * J_NOT_NULL vt) const noexcept {
      return kind == term_expr_type_kind::val_type && value_type == vt;
    }

    [[nodiscard]] bool operator==(const attr_type_def & at) const noexcept;

    J_A(AI,NODISC,HIDDEN) inline operator bool() const noexcept {
      return kind != term_expr_type_kind::none;
    }

    J_A(AI,NODISC,HIDDEN) inline bool operator!() const noexcept {
      return kind == term_expr_type_kind::none;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_any() const noexcept {
      return kind == term_expr_type_kind::any;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_attr_value() const noexcept {
      return kind == term_expr_type_kind::attr_value_type;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_nt() const noexcept {
      return kind == term_expr_type_kind::nt;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_nt_type() const noexcept {
      return kind == term_expr_type_kind::nt_type;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_any_nt() const noexcept {
      return kind == term_expr_type_kind::nt_type || kind == term_expr_type_kind::nt;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_val_type() const noexcept {
      return kind == term_expr_type_kind::val_type;
    }

    J_A(AI,NODISC,HIDDEN) inline bool is_term_or_terms() const noexcept {
      return kind == term_expr_type_kind::term_or_terms;
    }

    J_A(RNN) [[nodiscard]] nt_data_type * get_nt_type() const noexcept;
    [[nodiscard]] u32_t get_nt_size() const noexcept;

    te_type unify(const te_type & rhs) const noexcept;

    J_A(AI) inline ~te_type() {
      if (kind == term_expr_type_kind::attr_value_type) {
        clear();
      }
    }

    term_expr_type_kind kind = term_expr_type_kind::none;
    union {
      val_type * value_type;
      attr_type_def attr_value_type;
      nt_data_type * nt_type;
      nt_p nt;
      u32_t size;
    };

    static const te_type term_or_terms;
    static const te_type any;
  };

  enum J_A(FLAGS) te_fn_arg_flags : u8_t {
    no_fn_arg_flags    = 0b0000000,
    arg_variadic       = 0b0000001,
    arg_written        = 0b0000010,
    lifecycle_consume  = 0b0000100,
    lifecycle_move_src = 0b0001000,
    lifecycle_move_dst = 0b0010000,
    lifecycle_use      = 0b0100000,
    lifecycle_use_mask = 0b0100000,
    lifecycle_returned = 0b1000000,
  };

  enum J_A(FLAGS) te_fn_flags : u8_t {
    no_fn_flags            = 0b0000,
    fn_variadic            = 0b0001,
    lifecycle_acquire      = 0b0010,
    lifecycle_returns_this = 0b0100,
  };

  struct te_function_arg final {
    te_type type;
    te_fn_arg_flags flags = no_fn_arg_flags;
    strings::string name;
    J_BOILERPLATE(te_function_arg, CTOR_NE_ND)

    template<typename T, typename S>
    J_A(AI,ND,HIDDEN) inline te_function_arg(T && type, S && name, te_fn_arg_flags flags = no_fn_arg_flags) noexcept
      : type(static_cast<T &&>(type)),
      flags(flags),
      name(static_cast<S &&>(name))
    { }

    J_A(AI,NODISC) inline bool is_written() const noexcept { return flags & arg_written; }
    J_A(AI,NODISC) inline bool is_move_src() const noexcept { return flags & lifecycle_move_src; }
    J_A(AI,NODISC) inline bool is_move_dst() const noexcept { return flags & lifecycle_move_dst; }
    J_A(AI,NODISC) inline bool consumes_arg() const noexcept { return flags & lifecycle_consume; }
    J_A(AI,NODISC) inline bool returns_arg() const noexcept { return flags & lifecycle_returned; }
    J_A(AI,NODISC) inline bool is_variadic() const noexcept { return flags & arg_variadic; }
  };

  using te_fn_args = noncopyable_vector<te_function_arg>;

  struct te_function final {
    J_BOILERPLATE(te_function, CTOR_NE_ND, EQ)

    template<typename Type>
    inline te_function(Type && type, codegen_template && tpl, te_fn_args && args, te_fn_flags flags = no_fn_flags, i32_t precedence = 100) noexcept
      : return_type(static_cast<Type &&>(type)),
        flags(flags),
        precedence(precedence),
        args(static_cast<te_fn_args &&>(args)),
        tpl(static_cast<codegen_template &&>(tpl))
    { }

    J_A(AI,NODISC) inline bool is_variadic() const noexcept { return flags & fn_variadic; }

    J_A(AI,NODISC) inline bool acquires_result() const noexcept { return flags & lifecycle_acquire; }
    J_A(AI,NODISC) inline bool returns_this() const noexcept { return flags & lifecycle_returns_this; }

    te_type return_type;
    te_fn_flags flags = no_fn_flags;
    i32_t precedence = 100;
    te_fn_args args{};
    codegen_template tpl{};
  };

  struct te_value_source;

  struct te_fn_set final {
    noncopyable_vector<te_function> fns{};
    [[nodiscard]] const te_function * maybe_find_overload(span<te_type> arg_types) const noexcept;
    [[nodiscard]] const te_function * maybe_find_overload(span<te_value_source> args) const noexcept;
    [[nodiscard]] const te_function & find_overload(span<te_type> arg_types) const noexcept;
    [[nodiscard]] const te_function & find_overload(span<te_value_source> args) const noexcept;

    template<typename... Ts>
    J_A(AI,HIDDEN,ND) inline te_function & add(Ts && ... args) {
      return fns.emplace_back(static_cast<Ts &&>(args)...);
    }
  };

  enum class te_source_type {
    none,
    opaque,
    constant,
    converted_constant,
    stack_term,
    stack_nt,
    stack_nt_field,
    construct_term,
    select_term_ahead,
  };


  struct te_term_ahead {
    i32_t offset = 0;
  };

  struct te_stack_value {
    i32_t stack_offset = 0;
  };

  struct te_stack_field {
    i32_t stack_offset = 0;
    i32_t field_offset = 0;
  };

  struct te_construct_term {
    term * term = nullptr;
    te_value_source *  dyn_term_type = nullptr;
    noncopyable_vector<te_value_source> in_args{};
    noncopyable_vector<te_value_source> const_args{};
  };

  struct te_value_source {
    J_A(AI) inline te_value_source() noexcept
      : source_type(te_source_type::none),
        dummy{}
    { }

    inline te_value_source(te_value_source && rhs) noexcept
      : source_type(rhs.source_type),
        precedence(rhs.precedence),
        type(static_cast<te_type &&>(rhs.type)),
        value(static_cast<attr_value &&>(rhs.value))
    {
      if (source_type == te_source_type::construct_term) {
        ::new (&construct_term) te_construct_term(static_cast<te_construct_term &&>(rhs.construct_term));
      } else {
        j::memcpy(&construct_term, &rhs.construct_term, sizeof(te_construct_term));
      }
      rhs.source_type = te_source_type::none;
    }

    inline te_value_source & operator=(te_value_source && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (source_type == te_source_type::construct_term) {
          clear();
        }
        source_type = rhs.source_type;
        precedence = rhs.precedence;
        type = static_cast<te_type &&>(rhs.type);
        value = static_cast<attr_value &&>(rhs.value);
        if (source_type == te_source_type::construct_term) {
          ::new (&construct_term) te_construct_term(static_cast<te_construct_term &&>(rhs.construct_term));
        } else {
          j::memcpy(&construct_term, &rhs.construct_term, sizeof(te_construct_term));
        }
        rhs.source_type = te_source_type::none;
      }
      return *this;
    }

    template<typename Av>
    inline te_value_source(te_source_type source_type, Av && value) noexcept
      : source_type(source_type),
        type(value),
        value{static_cast<Av &&>(value)},
        dummy{}
    { }

    template<typename T>
    inline te_value_source(te_source_type source_type, T && type, attr_value && value) noexcept
      : source_type(source_type),
        type(static_cast<T &&>(type)),
        value{static_cast<attr_value &&>(value)},
        dummy{}
    { }

    template<typename T>
    inline te_value_source(te_source_type source_type, T && type, attr_value && value, i32_t precedence) noexcept
      : source_type(source_type),
        precedence(precedence),
        type(static_cast<T &&>(type)),
        value{static_cast<attr_value &&>(value)},
        dummy{}
    { }

    template<typename T>
    inline te_value_source(te_source_type source_type, T && type, attr_value && value, i32_t precedence, attr_value && original) noexcept
      : source_type(source_type),
        precedence(precedence),
        type(static_cast<T &&>(type)),
        value{static_cast<attr_value &&>(value)},
        original{static_cast<attr_value &&>(original)}
    { }

    template<typename T, typename Sv>
    J_A(AI) inline te_value_source(te_source_type source_type, T && type, Sv && stack_value) noexcept
      : source_type(source_type),
        type(static_cast<T &&>(type)),
        stack_value{static_cast<Sv &&>(stack_value)}
    { }

    template<typename T>
    J_A(AI) inline te_value_source(te_source_type source_type, T && type, const te_stack_field & stack_field) noexcept
      : source_type(source_type),
        type(static_cast<T &&>(type)),
        stack_field{stack_field}
    { }

    template<typename T>
    J_A(AI) inline te_value_source(T && type, const te_term_ahead & term_ahead) noexcept
      : source_type(te_source_type::select_term_ahead),
        type(static_cast<T &&>(type)),
        term_ahead{term_ahead}
    { }

    template<typename T>
    J_A(AI) inline te_value_source(te_source_type source_type, T && type, te_construct_term && construct) noexcept
      : source_type(source_type),
        type(static_cast<T &&>(type)),
        construct_term{static_cast<te_construct_term &&>(construct)}
    { }

    void clear() noexcept;

    ~te_value_source() {
      if (source_type == te_source_type::construct_term) {
        clear();
      }
    }

    [[nodiscard]] strings::string as_str(i32_t precedence) const noexcept;

    te_source_type source_type;
    i32_t precedence = 0;
    te_type type;
    attr_value value;

    union {
      struct {} dummy;
      te_stack_value stack_value;
      te_term_ahead term_ahead;
      te_stack_field stack_field;
      te_construct_term construct_term;
      attr_value original;
    };
  };

  using term_exprs = vector<term_expr*>;
  [[nodiscard]] bool operator==(const term_exprs & lhs, const term_exprs & rhs) noexcept;

  struct te_term_construct final {
    term * term = nullptr;
    term_expr * term_type_expr = nullptr;
    term_exprs args{};
    inline te_term_construct() noexcept = default;

    template<typename... Ts>
    explicit te_term_construct(struct term * J_NOT_NULL term, Ts && ... args) noexcept
      : term(term),
        args(static_cast<Ts &&>(args)...)
    { }

    template<typename... Ts>
    explicit te_term_construct(term_expr * J_NOT_NULL term_type_expr, Ts && ... args) noexcept
      : term_type_expr(term_type_expr),
        args(static_cast<Ts &&>(args)...)
    { }
    [[nodiscard]] bool operator==(const te_term_construct & rhs) const noexcept;
  };

  struct te_prop_read final {
    term_expr * val = nullptr;
    strings::string prop_name{};
    [[nodiscard]] bool operator==(const te_prop_read & rhs) const noexcept;
  };

  struct te_constant final {
    expr * init_expr = nullptr;
    attr_value value{};
    val_type * type = nullptr;
  };

  struct te_member_fn_call final {
    term_expr * val = nullptr;
    strings::string fn_name{};
    term_exprs args{};
    [[nodiscard]] bool operator==(const te_member_fn_call & rhs) const noexcept;
    [[nodiscard]] const te_function & find_overload(expr_scope & scope) const;
  };

  struct te_global_fn_call final {
    strings::string fn_name{};
    term_exprs args{};
    [[nodiscard]] bool operator==(const te_global_fn_call & rhs) const noexcept;
    [[nodiscard]] const te_function & find_overload(expr_scope & scope) const;
  };

  struct te_ternary final {
    term_expr * condition = nullptr;
    term_expr * true_branch = nullptr;
    term_expr * false_branch = nullptr;
    [[nodiscard]] bool operator==(const te_ternary & rhs) const noexcept;
  };

  struct te_unop final {
    operator_type type;
    term_expr * val = nullptr;
    [[nodiscard]] bool operator==(const te_unop & rhs) const noexcept;
  };

  struct te_binop final {
    operator_type type;
    term_expr * lhs = nullptr;
    term_expr * rhs = nullptr;
    [[nodiscard]] bool operator==(const te_binop & rhs) const noexcept;
  };

  struct arg_counts;
  struct arg_precompute;

  struct precomputes;

  struct term_expr final {
    term_expr_type type = term_expr_type::none;

    J_A(AI) inline term_expr() noexcept : dummy{} {}

    J_A(AI) inline term_expr(const te_binop & binop) noexcept
      : type(term_expr_type::binop),
        binop{binop}
    { }

    J_A(AI) inline term_expr(const reduction_arg & arg) noexcept
      : type(term_expr_type::arg),
        arg{arg}
    { J_ASSERT(arg); }

    J_A(AI) inline term_expr(reduction_arg && arg) noexcept
      : type(term_expr_type::arg),
        arg{static_cast<reduction_arg &&>(arg)}
    { J_ASSERT(this->arg); }

    J_A(AI) inline term_expr(te_term_construct && v) noexcept
    : type(term_expr_type::term_construct),
       term_construct{static_cast<te_term_construct &&>(v)}
    { }

    J_A(AI) inline term_expr(expr * J_NOT_NULL const_init) noexcept
    : type(term_expr_type::constant),
      constant{const_init, {}}
    { }

    J_A(AI) explicit inline term_expr(attr_value && value) noexcept
    : type(term_expr_type::constant),
      constant{nullptr, static_cast<attr_value&&>(value)}
    { }

    J_A(AI) inline term_expr(attr_value && value, val_type * J_NOT_NULL val_type) noexcept
    : type(term_expr_type::constant),
      constant{nullptr, static_cast<attr_value&&>(value), val_type}
    { }

    J_A(AI) inline term_expr(te_constant && v) noexcept
    : type(term_expr_type::constant),
      constant{static_cast<te_constant &&>(v)}
    {  }

    J_A(AI) inline term_expr(te_prop_read && v) noexcept
    : type(term_expr_type::prop_read),
      prop_read(static_cast<te_prop_read &&>(v))
    { }

    J_A(AI) inline term_expr(te_member_fn_call && v) noexcept
    : type(term_expr_type::member_fn_call),
      member_fn_call(static_cast<te_member_fn_call &&>(v))
    { }

    J_A(AI) inline term_expr(te_global_fn_call && v) noexcept
    : type(term_expr_type::global_fn_call),
      global_fn_call(static_cast<te_global_fn_call &&>(v))
    { }

    J_A(AI) inline term_expr(te_ternary && v) noexcept
    : type(term_expr_type::ternary),
      ternary(static_cast<te_ternary &&>(v))
    { }

    void resolve(const pat_captures & caps, expr_scope & scope,
                 id_resolver * J_NOT_NULL id_resolver,
                 term_expr_use use = term_expr_use::normal);

    void resolve(expr_scope & scope,
                 id_resolver * J_NOT_NULL id_resolver,
                 term_expr_use use = term_expr_use::normal);

    void compute_hash(expr_scope & scope);

    [[nodiscard]] arg_counts collect_args() const noexcept;

    void clear() noexcept;

    [[nodiscard]] attr_value compile(expr_scope & scope, precomputes & precompiles, te_type type);
    [[nodiscard]] attr_value compile(expr_scope & scope, precomputes & precompiles);
    [[nodiscard]] attr_value compile(expr_scope & scope, te_type type);
    [[nodiscard]] attr_value compile(expr_scope & scope);
    [[nodiscard]] strings::string compile_str(expr_scope & scope, precomputes & precompiles, te_type type);
    [[nodiscard]] strings::string compile_str(expr_scope & scope, precomputes & precompiles);
    [[nodiscard]] strings::string compile_str(expr_scope & scope, te_type type);
    [[nodiscard]] strings::string compile_str(expr_scope & scope);
    [[nodiscard]] te_value_source compile_raw(expr_scope & scope, precomputes & precompiles);
    [[nodiscard]] te_value_source compile_raw(expr_scope & scope, precomputes & precompiles, te_type type);

    [[nodiscard]] pair<i32_t> extent() const noexcept;

    [[nodiscard]] te_type get_type(expr_scope & scope);

    void clear_arg_nt(term_expr_use use) noexcept;

    J_A(AI) inline ~term_expr() {
      if (type != term_expr_type::none) {
        clear();
      }
    }

    [[nodiscard]] bool operator==(const term_expr & rhs) const noexcept;

    union {
      struct {} dummy;
      reduction_arg arg;
      te_term_construct term_construct;
      te_constant constant;
      te_prop_read prop_read;
      te_member_fn_call member_fn_call;
      te_ternary ternary;
      te_binop binop;
      te_global_fn_call global_fn_call;
    };

    u32_t hash = 0U;
    source_location loc{};
  };

  struct precompute {
    strings::string name;
    strings::string type;
    strings::string value;
  };

  struct precomputes {
    hash_map<reduction_arg, precompute*, reduction_arg_hash> args;
    hash_map<term_expr*, precompute*> tes;
    noncopyable_vector<precompute*> all;

    precompute * find_precompute(term_expr * J_NOT_NULL te) noexcept;
    precompute * find_precompute(const reduction_arg & arg) noexcept;
    void add_precompute(const reduction_arg & arg, strings::string && name, strings::string && type);
    void add_precompute(const reduction_arg & arg, strings::string && name, strings::string && type, strings::string && value) noexcept;
    void add_precompute(term_expr * J_NOT_NULL te, strings::string && name, strings::string && type);
    void add_precompute(term_expr * J_NOT_NULL te, strings::string && name, strings::string && type, strings::string && value) noexcept;
  };

  struct arg_counts final {
    hash_map<reduction_arg, i32_t, reduction_arg_hash> counts;
    void add(const reduction_arg & arg, i32_t num = 1) noexcept;
    arg_counts & operator+=(const arg_counts & rhs) noexcept;
    void filter_min(i32_t min) noexcept;
    [[nodiscard]] inline u32_t size() const noexcept { return counts.size(); }
    [[nodiscard]] inline auto begin() const noexcept { return counts.begin(); }
    [[nodiscard]] inline auto end() const noexcept { return counts.end(); }
    [[nodiscard]] precomputes as_precomputes(const module & mod) const noexcept;
  };

  template<typename Te, typename Fn, typename... Ts>
  J_A(AI,HIDDEN,ND) inline decltype(auto) visit(Te * J_NOT_NULL te, Fn && fn, Ts && ... ts) {
    J_SCOPE_INFO(source_loc(te->loc));
    using enum term_expr_type;
    switch (te->type) {
    case none: J_FAIL("Visited empty te_expr");
    case arg: return (static_cast<Fn &&>(fn))(te->arg, static_cast<Ts &&>(ts)...);
    case term_construct: return (static_cast<Fn &&>(fn))(te->term_construct, static_cast<Ts &&>(ts)...);
    case constant: return (static_cast<Fn &&>(fn))(te->constant, static_cast<Ts &&>(ts)...);
    case prop_read: return (static_cast<Fn &&>(fn))(te->prop_read, static_cast<Ts &&>(ts)...);
    case member_fn_call: return (static_cast<Fn &&>(fn))(te->member_fn_call, static_cast<Ts &&>(ts)...);
    case global_fn_call: return (static_cast<Fn &&>(fn))(te->global_fn_call, static_cast<Ts &&>(ts)...);
    case ternary: return (static_cast<Fn &&>(fn))(te->ternary, static_cast<Ts &&>(ts)...);
    case binop: return (static_cast<Fn &&>(fn))(te->binop, static_cast<Ts &&>(ts)...);
    }
  }

  template<typename Te, typename Fn, typename... Ts>
  J_A(AI,HIDDEN,ND) inline decltype(auto) visit_with_expr(Te * J_NOT_NULL te, Fn && fn, Ts && ... ts) {
    return visit(te, static_cast<Fn &&>(fn), static_cast<Ts &&>(ts)..., te);
  }

  template<typename Tes, typename Fn, typename... Ts>
  J_A(AI,HIDDEN,ND) inline void visit(Tes & tes, Fn && fn, Ts && ... ts) {
    for (auto te : tes) {
      visit(te, static_cast<Fn &&>(fn), static_cast<Ts &&>(ts)...);
    }
  }

  template<typename Tes, typename Fn, typename... Ts>
  J_A(AI,HIDDEN,ND) inline void visit_with_expr(Tes & tes, Fn && fn, Ts && ... ts) {
    for (auto te : tes) {
      visit(te, static_cast<Fn &&>(fn), static_cast<Ts &&>(ts)..., te);
    }
  }

  void dump(dump_context & ctx, const term_expr * e) noexcept;

  using copy_cb_t = void (*)(const term_expr * J_NOT_NULL, term_expr * J_NOT_NULL, void *) noexcept;
  [[nodiscard]] term_expr * copy(const term_expr * J_NOT_NULL te, copy_cb_t cb, void * userdata, source_location loc = {}) noexcept;
  J_A(AI,ND,NODISC,HIDDEN) inline term_expr * copy(const term_expr * J_NOT_NULL te, source_location loc = {}) noexcept {
    return copy(te, nullptr, nullptr, loc);
  }
}
