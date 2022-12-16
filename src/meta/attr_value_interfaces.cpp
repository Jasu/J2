#include "attr_value_interfaces.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/rules/rule.hpp"
#include "strings/format.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/attr_value_hash.hpp"
#include "strings/string.hpp"
#include "containers/hash_map.hpp"
#include "meta/expr_scope.hpp"
#include "streams/string_sink.hpp"
#include "meta/attr_value_interface_def.hpp"
#include "meta/attr_context.hpp"
#include "meta/value_types.hpp"
#include "meta/module.hpp"
#include "meta/term.hpp"

namespace j::meta {
  namespace s = strings;

  namespace {
    bool is_int(const attr_value & v) noexcept {
      return v.type == attr_int;
    }
    bool is_tuple(const attr_value & v) noexcept {
      return v.type == attr_tuple;
    }
    bool is_string(const attr_value & v) noexcept {
      return v.type == attr_str;
    }
    bool is_enum(const attr_value & v) noexcept {
      return v.type == attr_enum;
    }
    bool is_id(const attr_value & v) noexcept {
      return v.type == attr_id;
    }
    J_A(NODESTROY) const attr_value_interface base_interface((property[]){
        make_getter<&is_int> ("IsInt"),
        make_getter<&is_tuple> ("IsTuple"),
        make_getter<&is_string>("IsString"),
        make_getter<&is_enum>("IsEnum"),
        make_getter<&is_id>("IsId"),
      },
      {});
    s::const_string_view get_doc_title(const attr_value & n) noexcept {
      return n.node->comment.title();
    }
    s::const_string_view get_doc_summary(const attr_value & n) noexcept {
      return n.node->comment.summary();
    }
    s::const_string_view get_doc_body(const attr_value & n) noexcept {
      return n.node->comment.body();
    }

    bool has_attr(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return name && name[0] == '@' && obj.node->attrs.has(name.without_prefix(1U));
    }

    attr_value get_attr(const expr_scope & s, const attr_value & obj, strings::const_string_view name) noexcept {
      return obj.node->attrs.at(*s.root->mod->attr_ctx, name.without_prefix(1U));
    }

    void set_attr(expr_scope & s, attr_value & obj, strings::const_string_view name, attr_value val) noexcept {
      obj.node->attrs.set(s.root->mod->attr_ctx, name.without_prefix(1U), static_cast<attr_value &&>(val));
    }

    bool has_member(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return reinterpret_cast<term*>(obj.node)->has_member(name);
    }

    attr_value get_member(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return attr_value(reinterpret_cast<term*>(obj.node)->get_member(name));
    }

    bool has_enumerant(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return reinterpret_cast<const enum_def*>(obj.node)->has(name);
    }

    attr_value get_enumerant(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      const enum_def * ed = reinterpret_cast<const enum_def*>(obj.node);
      return {ed, ed->at(name)};
    }

    void set_enumerant(expr_scope &, attr_value & obj, strings::const_string_view name, attr_value val) noexcept {
      enum_def * ed = reinterpret_cast<enum_def*>(obj.node);
      if (val) {
        ed->add(name, val.as_int());
      } else {
        ed->add(name);
      }
    }

    bool has_enum_size(const attr_value & e) noexcept {
      return e.enum_val.def->is_flag_enum;
    }
    i64_t get_enum_size(const attr_value & e) noexcept {
      J_REQUIRE(e.enum_val.def->is_flag_enum, "Size of a non-flag enum.");
      return bits::popcount(e.enum_val.value);
    }
    template<bool Full>
    s::string get_enum_name(const attr_value & e) noexcept {
      return e.enum_val.def->get_name(e.enum_val.value, Full);
    }

    u64_t get_enum_val(const attr_value & e) noexcept {
      return e.enum_val.value;
    }

    J_A(NODESTROY) const attr_value_interface enum_interface(
      (property[]){
        make_getter<&get_enum_size, &has_enum_size> ("Size"),
        make_getter<&get_enum_name<false>>          ("Name"),
        make_getter<&get_enum_name<true>>           ("FullName"),
        make_getter<&get_enum_val>                  ("Int"),
      }, {}, &base_interface);

    J_A(NODESTROY) const attr_value_interface node_interface((property[]){
        make_getter<&is_int> ("IsInt"),
        make_getter<&is_tuple> ("IsTuple"),
        make_getter<&is_string>("IsString"),
        make_getter<&is_enum>("IsEnum"),
        make_getter<&is_id>("IsId"),
        make_prop_ro<&node::name>                  ("Name"),
        make_prop_ro<&node::index>                 ("Index"),
        make_getter<&get_doc_title>                ("DocTitle"),
        make_getter<&get_doc_summary>              ("DocSummary"),
        make_getter<&get_doc_body>                 ("DocBody"),
      },
      {},
      nullptr,
      &has_attr, &get_attr, &set_attr);

    [[nodiscard]] trivial_array_copyable<attr_value> to_tuple(span<term_member* const> members) noexcept {
      trivial_array_copyable<attr_value> values{containers::uninitialized, members.size()};
      for (u32_t i = 0U, sz = members.size(); i < sz; ++i) {
        values.initialize_element(members[i]);
      }
      return static_cast<trivial_array_copyable<attr_value> &&>(values);
    }

    [[nodiscard]] i64_t term_num_members(const term & t) noexcept {
      return t.num_inputs + t.num_consts + t.num_data;
    }
    [[nodiscard]] i64_t term_num_static_inputs(const term & t) noexcept {
      return t.num_inputs ? t.num_inputs - t.inputs().back()->has_dynamic_size : 0U;
    }
    [[nodiscard]] i64_t term_num_operands(const term & t) noexcept {
      return t.num_inputs + t.num_consts;
    }
    [[nodiscard]] attr_value term_inputs(term & t) noexcept {
      return attr_value{to_tuple(t.inputs())};
    }
    [[nodiscard]] attr_value term_consts(term & t) noexcept {
      return attr_value{to_tuple(t.consts())};
    }
    [[nodiscard]] attr_value term_data(term & t) noexcept {
      return attr_value{to_tuple(t.data())};
    }
    [[nodiscard]] attr_value term_members(term & t) noexcept {
      return attr_value{to_tuple(t.stored())};
    }
    [[nodiscard]] attr_value term_ctor(term & t) noexcept {
      return attr_value{to_tuple(t.operands())};
    }

    J_A(NODESTROY) const attr_value_interface term_interface(
      (property[]){
        make_prop_ro<&term::num_inputs>                  ("NumInputs"),
        make_prop_ro<&term::num_consts>                  ("NumConsts"),
        make_prop_ro<&term::num_data>                    ("NumData"),
        make_prop_ro<&term::has_dynamic_input>           ("HasDynamicInput"),
        make_prop_ro<&term::has_dynamic_const>           ("HasDynamicConst"),
        make_prop_ro<&term::has_dynamic_data>            ("HasDynamicData"),
        make_prop_ro<&term::static_size>                 ("StaticSize"),
        make_prop_ro<&term::static_input_size>           ("StaticInputSize"),
        make_prop_ro<&term::static_const_size>           ("StaticConstSize"),
        make_prop_ro<&term::static_data_size>            ("StaticDataSize"),
        make_prop_ro<&term::has_dynamic_size>            ("HasDynamicSize"),
        make_getter<&term_num_members>                   ("NumMembers"),
        make_getter<&term_num_operands>                  ("NumOperands"),
        make_getter<&term_num_static_inputs>             ("NumStaticInputs"),
        make_getter<&term_inputs>                        ("Inputs"),
        make_getter<&term_consts>                        ("Consts"),
        make_getter<&term_data>                          ("Data"),
        make_getter<&term_members>                       ("Members"),
        make_getter<&term_ctor>                          ("Ctor"),
      },
      {},
      &node_interface,
      &has_member, &get_member);

    J_A(NODESTROY) const attr_value_interface term_member_interface(
                                                            (property[]){
        make_prop_ro<&term_member::type>                    ("Type"),
        make_prop_ro<&term_member::static_offset>           ("StaticOffset"),
        make_prop_ro<&term_member::has_dynamic_size>        ("HasDynamicSize"),

        make_prop_ro<&term_member::ctor_index>              ("CtorIndex"),
        make_prop_ro<&term_member::region_index>            ("RegionIndex"),

        make_getter<&term_member::has_dynamic_offset>       ("HasDynamicOffset"),
        make_getter<&term_member::get_dynamic_offset_member>("DynamicOffsetMember"),

        make_getter<&term_member::size>                     ("Size"),
        make_getter<&term_member::size_expr>                ("SizeExpr"),
        make_getter<&term_member::ctor_size_expr>           ("CtorSizeExpr"),
        make_getter<&term_member::align>                    ("Align"),
        make_getter<&term_member::is_scalar>                ("IsScalar"),

        make_getter<&term_member::ctor_stmt>                ("CtorStmt"),
        make_getter<&term_member::getter_expr>              ("GetterExpr"),
        make_getter<&term_member::const_getter_expr>        ("ConstGetterExpr"),

        make_getter<&term_member::type_name>                ("TypeName"),
        make_getter<&term_member::ctor_type_name>           ("CtorTypeName"),
        make_getter<&term_member::getter_type_name>         ("GetterTypeName"),
        make_getter<&term_member::const_getter_type_name>   ("ConstGetterTypeName"),

        make_getter<&term_member::is_input>                 ("IsInput"),
        make_getter<&term_member::is_const>                 ("IsConst"),
        make_getter<&term_member::is_data>                  ("IsData"),
      },
      {},
      &node_interface);


    attr_value create_val_type(expr_scope & scope, const attr_value & self, const attr_value & val) {
      val_type & vt = self.as<val_type &>();
      for (auto & conv : vt.attr_conversions) {
        if (conv.first.matches(val)) {
          expr_scope sub(&scope);
          sub.set("It", attr_value(val));
          return attr_value(scope.root->mod->expand(conv.second, sub));
        }
      }
      scope.throw_expr_error(s::format("{}.Create does not support input value type {}.", vt.name, val.type));
    }

    attr_value val_type_val_eq_attr(expr_scope & scope, const attr_value & self, const attr_value & val, const attr_value & attr) {
      val_type & vt = self.as<val_type &>();
      if (!val.is_str()) {
        scope.throw_expr_error(s::format("{}.ValEqConst called with unsupported left-hand side.", vt.name));
      }
      term_expr * lhs = ::new term_expr(attr_value(val), &vt);
      term_expr * rhs = ::new term_expr(attr_value(attr));
      term_expr eq(te_binop{operator_type::eq, lhs, rhs});
      return attr_value(eq.compile_str(scope));
    }

    attr_value val_type_val_eq_val(expr_scope & scope, const attr_value & self, const attr_value & val, const attr_value & val_rhs) {
      val_type & vt = self.as<val_type &>();
      if (!val.is_str() || !val_rhs.is_str()) {
        scope.throw_expr_error(s::format("{}.ValEqVal called with non-string arg.", vt.name));
      }
      term_expr * lhs = ::new term_expr(attr_value(val), &vt);
      term_expr * rhs = ::new term_expr(attr_value(val_rhs), &vt);
      term_expr eq(te_binop{operator_type::eq, lhs, rhs});
      return attr_value(eq.compile_str(scope));
    }

    attr_value val_type_val_index_attr(expr_scope & scope, const attr_value & self, const attr_value & val, const attr_value & attr) {
      val_type & vt = self.as<val_type &>();
      if (!val.is_str()) {
        scope.throw_expr_error(s::format("{}.ValIndexConst called with unsupported left-hand side.", vt.name));
      }
      term_expr * lhs = ::new term_expr(attr_value(val), &vt);
      term_expr * rhs = ::new term_expr(attr_value(attr));
      term_expr eq(te_binop{operator_type::index, lhs, rhs});
      return attr_value(eq.compile_str(scope));
    }

    J_A(NODESTROY) const attr_value_interface val_type_interface((property[]){
        make_prop<&val_type::size>                  ("Size"),
        make_prop<&val_type::align>                 ("Align"),
        make_prop<&val_type::type_name>             ("Type"),
        make_prop<&val_type::is_scalar>             ("Scalar"),
        make_prop<&val_type::is_trs_term_type>      ("IsTerm"),
        make_prop<&val_type::is_trs_terms_type>     ("IsTerms"),
        make_prop<&val_type::getter_type_name>      ("GetterType"),
        make_prop<&val_type::getter_expr>           ("GetterExpr"),
        make_prop<&val_type::const_getter_type_name>("ConstGetterType"),
        make_prop<&val_type::const_getter_expr>     ("ConstGetterExpr"),
        make_prop<&val_type::size_expr>             ("SizeExpr"),
        make_prop<&val_type::ctor_type_name>        ("CtorType"),
        make_prop<&val_type::ctor_stmt>             ("CtorStmt"),
        make_prop<&val_type::ctor_size_expr>        ("CtorSizeExpr"),

        make_prop<&val_type::format_expr>           ("Format"),
        make_prop<&val_type::default_value_expr>    ("DefaultValue"),
        make_prop<&val_type::default_name_expr>     ("DefaultName"),

        make_prop<&val_type::default_name_expr>     ("DefaultName"),
      },
      (pair<s::const_string_view, expr_fn>[]){
        {"Create", make_expr_fn(create_val_type)},
        {"ValEqConst", make_expr_fn(val_type_val_eq_attr)},
        {"ValEqVal", make_expr_fn(val_type_val_eq_val)},
        {"ValIndexConst", make_expr_fn(val_type_val_index_attr)},
      },
      &node_interface);

    J_A(NODESTROY) const attr_value_interface attr_def_interface((property[]){
        make_prop<&attr_def::default_get_value>     ("DefaultGet"),
        make_prop<&attr_def::default_set_value>     ("DefaultSet"),
      },
      {},
      &node_interface);

    J_A(NODESTROY) const attr_value_interface enum_def_interface(
      (property[]){
        make_getter<&enum_def::size>                  ("Size"),
      }, {},
      &node_interface,
      &has_enumerant, &get_enumerant, &set_enumerant);

    J_A(NODESTROY) const attr_value_interface rule_interface(
      {}, {},
      &node_interface);

    J_A(NODESTROY) const attr_value_interface nonterminal_interface(
      {}, {},
      &node_interface);


    bool node_set_has(const attr_value & set, const attr_value & n) {
      return node_set_node_attr_type(set.type) == n.type && set.node_set.has(*n.node);
    }

    attr_value node_set_first(const expr_scope & scope, const attr_value & set) noexcept {
      return attr_value(&*set.as_node_set().iterate(scope.root->mod).begin());
    }
    bool node_set_has_one(const expr_scope &, const attr_value & set) noexcept {
      return (bool)set.as_node_set();
    }

    J_A(NODESTROY) const attr_value_interface node_set_interface(
      (property[]){
        make_getter<&basic_node_set::size> ("Size"),
        property{"First", &node_set_first, &node_set_has_one},
      },
      (pair<s::const_string_view, expr_fn>[]){
        make_method("Includes", &node_set_has),
      }, &base_interface);

    template<bool Right>
    [[nodiscard]] static s::string pad_space(s::const_string_view s, i64_t width) {
      const i32_t sz = s.size();
      if (width <= sz) {
        return s;
      }
      s::string str{(i32_t)width, ' '};
      char * wr = str.begin();
      if constexpr (!Right) {
        wr += width - sz;
      }
      ::j::memcpy(wr, s.begin(), sz);
      return str;
    }

    template<bool Right>
    [[nodiscard]] static s::string pad(s::const_string_view s, i64_t width, s::string ch) {
      const u32_t sz = s.size();
      if (width <= sz) {
        return s;
      }
      J_REQUIRE(ch.size() == 1U, "Expected a single-character string in pad.");
      s::string str{(i32_t)width, ch[0]};
      char * wr = str.begin();
      if constexpr (!Right) {
        wr += width - sz;
      }
      ::j::memcpy(wr, s.begin(), sz);
      return str;
    }

    template<bool Left, bool Right>
    s::const_string_view trim_default(s::const_string_view s) {
      const char * c = s.begin(), *e = s.end();
      if constexpr (Left) {
        while (c != e && j::strchr(" \n\t\r", *c)) {
          ++c;
        }
      }
      if constexpr (Right) {
        while (c != e && j::strchr(" \n\t\r", e[-1])) {
          --e;
        }
      }
      return {c, e};
    }

    template<bool Left, bool Right>
    s::const_string_view trim(s::const_string_view s, s::string m) {
      const char * mask = m ? m.data() : " \n\t\r";
      const char * c = s.begin(), *e = s.end();
      if constexpr (Left) {
        while (c != e && j::strchr(mask, *c)) {
          ++c;
        }
      }
      if constexpr (Right) {
        while (c != e && j::strchr(mask, e[-1])) {
          --e;
        }
      }
      return {c, e};
    }

    s::string repeat(s::const_string_view s, i64_t n) {
      J_REQUIRE(n >= 0, "Repeat called with negative repeat.");
      s::string result;
      while (n--) {
        result += s;
      }
      return result;
    }

    J_A(NODESTROY) const attr_value_interface str_interface(
      (property[]){
        make_getter<&s::string::size>                  ("Size"),
      },
      (pair<s::const_string_view, expr_fn>[]){
        make_method("Repeat", repeat),
        make_method("Trim", &trim<true, true>),
        make_method("Ltrim", &trim<true, false>),
        make_method("Rtrim", &trim<false, true>),
        make_method("Trim", &trim_default<true, true>),
        make_method("Ltrim", &trim_default<true, false>),
        make_method("Rtrim", &trim_default<false, true>),
        make_method("PadLeft", &pad_space<false>),
        make_method("PadLeft", &pad<false>),
        make_method("PadRight", &pad_space<true>),
        make_method("PadRight", &pad<true>),
      }, &base_interface);

    s::string format_id(lisp::id_name_ref n) {
      return n.pkg + ":" + n.name;
    }

    J_A(NODESTROY) const attr_value_interface id_interface(
      (property[]){
        make_prop<&lisp::id_name::pkg>("Pkg"),
        make_prop<&lisp::id_name::name>("Name"),
        make_prop<&lisp::id::raw>("Raw"),
      },
      (pair<strings::const_string_view, expr_fn>[]){
        make_method("ToString", &format_id),
      }, &base_interface);

    attr_value expand_tpl(expr_scope & scope, const attr_value & s) {
      j::mem::shared_ptr<streams::string_sink> str(j::mem::make_shared<streams::string_sink>());
      s::formatted_sink to(j::mem::static_pointer_cast<streams::sink>(str));
      code_writer w(&to);
      s.tpl->expand(w, scope);
      to.flush();
      return attr_value(static_cast<s::string &&>(str->string));
    }

    J_A(NODESTROY) const attr_value_interface tpl_interface(
      {},
      (pair<s::const_string_view, expr_fn>[]){
        {"Expand", make_expr_fn(expand_tpl)},
      }, &base_interface);


    attr_value tuple_chunk(const attr_value & tuple, i64_t count) {
      J_REQUIRE(count > 0, "Tried to chunk tuple with a non-positive count '{}'.", count);
      const u32_t in_sz = tuple.tuple->size();
      const u32_t sz = (in_sz + count - 1) / count;
      attr_value result[sz];
      attr_value * wr = result;
      const attr_value * rd = tuple.tuple->begin();
      for (u32_t i = 0U; i < in_sz; i += count, rd += count, ++wr) {
        *wr = attr_value(trivial_array_copyable<attr_value>(containers::copy, rd, j::min(count, in_sz - i)));
      }
      return attr_value(span<attr_value>(result, sz));
    }

    bool tuple_includes(const attr_value & tuple, const attr_value & it) {
      for (auto & cand : *tuple.tuple) {
        if (cand == it) {
          return true;
        }
      }
      return false;
    }
    attr_value tuple_unique(const attr_value & tuple) {
      hash_set<attr_value, attr_value_hash> hs;
      attr_value to[tuple.tuple->size()];
      u32_t i = 0U;
      for (auto & cand : *tuple.tuple) {
        if (hs.emplace(cand).second) {
          to[i++] = cand;
        }
      }
      return attr_value(span<attr_value>(to, i));
    }

    attr_value tuple_slice(const attr_value & tuple, i64_t begin) {
      begin = min(begin, tuple.tuple->size());
      attr_value to[tuple.tuple->size() - begin];
      u32_t i = 0U;
      for (auto it = tuple.tuple->begin() + begin, end = tuple.tuple->end(); it != end; ++it) {
        to[i++] = *it;
      }
      return attr_value(span<attr_value>(to, i));
    }

    attr_value tuple_slice2(const attr_value & tuple, i64_t begin, i64_t len) {
      begin = min(begin, tuple.tuple->size());
      len = min(len, tuple.tuple->size() - begin);
      attr_value to[len];
      u32_t i = 0U;
      for (auto it = tuple.tuple->begin() + begin, end = it + len; it != end; ++it) {
        to[i++] = *it;
      }
      return attr_value(span<attr_value>(to, i));
    }

    attr_value tuple_append(const attr_value & a, const attr_value & b) {
      J_REQUIRE(b.type == attr_tuple, "Expected tuple in append");
      attr_value to[a.tuple->size() + b.tuple->size()];
      u32_t i = 0U;
      for (auto & e : *a.tuple) {
        to[i++] = e;
      }
      for (auto & e : *b.tuple) {
        to[i++] = e;
      }
      return attr_value(span<attr_value>(to, i));
    }

    attr_value tuple_pad_right(const attr_value & a, i32_t size, const attr_value & pad) {
      i32_t sz = a.tuple->size();
      if (sz >= size) {
        return a;
      }
      attr_value to[size];
      i32_t i = 0U;
      for (auto & e : *a.tuple) {
        ::new (to + i++) attr_value(e);
      }
      for (; i != size; ++i) {
        ::new (to + i) attr_value(pad);
      }
      return attr_value(span<attr_value>(to, size));
    }
    attr_value tuple_at(expr_scope & scope, const attr_value & tuple, i64_t index) {
      if (index >= 0 && index < tuple.tuple->size()) {
        return tuple.tuple->operator[](index);
      }
      scope.throw_expr_error(s::format("At({}) called for tuple with {} items.", index, tuple.tuple->size()));
    }
    attr_value tuple_maybe_at(const attr_value & tuple, i64_t index) {
      return tuple.tuple->size() > index ? tuple.tuple->at(index) : attr_value();
    }
    attr_value tuple_first(const attr_value & tuple) { return tuple.tuple->at(0); }
    attr_value tuple_second(const attr_value & tuple) { return tuple.tuple->at(1); }
    attr_value tuple_third(const attr_value & tuple) { return tuple.tuple->at(2); }
    attr_value tuple_fourth(const attr_value & tuple) { return tuple.tuple->at(3); }
    attr_value tuple_fifth(const attr_value & tuple) { return tuple.tuple->at(4); }
    attr_value tuple_last(const attr_value & tuple) { return tuple.tuple->at(tuple.tuple->size() - 1U); }
    bool tuple_has_one(const attr_value & tuple) noexcept { return tuple.tuple->size() >= 1U; }
    bool tuple_has_two(const attr_value & tuple) noexcept { return tuple.tuple->size() >= 2U; }
    bool tuple_has_three(const attr_value & tuple) noexcept { return tuple.tuple->size() >= 3U; }
    bool tuple_has_four(const attr_value & tuple) noexcept { return tuple.tuple->size() >= 4U; }
    bool tuple_has_five(const attr_value & tuple) noexcept { return tuple.tuple->size() >= 5U; }

    J_A(NODESTROY) const attr_value_interface tuple_interface(
      (property[]){
        make_getter<&trivial_array_copyable<attr_value>::size> ("Size"),
        make_getter<&tuple_first, tuple_has_one>("First"),
        make_getter<&tuple_second, tuple_has_two>("Second"),
        make_getter<&tuple_third, tuple_has_three>("Third"),
        make_getter<&tuple_fourth, tuple_has_four>("Fourth"),
        make_getter<&tuple_fifth, tuple_has_five>("Fifth"),
        make_getter<&tuple_last, tuple_has_one>("Last"),
      },
      (pair<s::const_string_view, expr_fn>[]){
        make_method("At", tuple_at),
        make_method("MaybeAt", tuple_maybe_at),
        make_method("Includes", tuple_includes),
        make_method("PadRight", tuple_pad_right),
        make_method("Chunk", tuple_chunk),
        make_method("Append", tuple_append),
        make_method("Unique", tuple_unique),
        make_method("Slice", tuple_slice),
        make_method("Slice", tuple_slice2),
      }, &base_interface);


    bool has_struct_val(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return obj.struct_val.has(name);
    }

    attr_value get_struct_val(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return obj.struct_val.at(name);
    }

    void set_struct_val(expr_scope &, attr_value & obj, strings::const_string_view name, attr_value val) noexcept {
      return obj.struct_val.set(name, static_cast<attr_value &&>(val));
    }

    J_A(NODESTROY) const attr_value_interface struct_interface(
      {}, {},
      &base_interface,
      &has_struct_val, &get_struct_val, &set_struct_val);
  }

    attr_value get_map_size(const attr_value & obj) noexcept {
      return attr_value(obj.map->size());
    }

    bool has_map_val(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      if (attr_value * v = obj.map->maybe_at(name)) {
        return (bool)*v;
      }
      return false;
    }

    attr_value get_map_val(const expr_scope &, const attr_value & obj, strings::const_string_view name) noexcept {
      return obj.map->at(name);
    }

  attr_value map_at(const attr_value & map, strings::const_string_view name) {
    return map.map->at(name);
  }

  void set_map_val(expr_scope &, attr_value & obj, strings::const_string_view name, attr_value val) noexcept {
    (*obj.map)[name] = static_cast<attr_value &&>(val);
  }

  J_A(NODESTROY) const attr_value_interface map_interface(
    (property[]){
      make_getter<&get_map_size> ("Size"),
    },
    (pair<s::const_string_view, expr_fn>[]){
      make_method("At", &map_at),
    },
    &base_interface,
    &has_map_val, &get_map_val, &set_map_val);

  const attr_value_interface* const interfaces[num_attr_types_v + 1U]{
    [(u8_t)attr_type_none] = &base_interface, // none
    [(u8_t)attr_int] = &base_interface, // int
    [(u8_t)attr_bool] = &base_interface, // bool
    [(u8_t)attr_enum] = &enum_interface,

    [(u8_t)attr_term] = &term_interface,
    [(u8_t)attr_term_member] = &term_member_interface,
    [(u8_t)attr_value_type] = &val_type_interface,
    [(u8_t)attr_attr_def] = &attr_def_interface,
    [(u8_t)attr_enum_def] = &enum_def_interface,
    [(u8_t)attr_struct_def] = &node_interface, //struct def
    [(u8_t)attr_nonterminal] = &nonterminal_interface,
    [(u8_t)attr_fn] = &node_interface, //fn

    [(u8_t)attr_gen_file_set] = &node_set_interface,
    [(u8_t)attr_term_set] = &node_set_interface,
    [(u8_t)attr_val_type_set] = &node_set_interface,

    [(u8_t)attr_rule] = &rule_interface,

    [(u8_t)attr_id] = &id_interface,
    [(u8_t)attr_str] = &str_interface,
    [(u8_t)attr_tpl] = &tpl_interface,
    [(u8_t)attr_tuple] = &tuple_interface,
    [(u8_t)attr_map] = &map_interface,
    [(u8_t)attr_struct] = &struct_interface,
  };
}
