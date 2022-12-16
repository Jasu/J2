#include "meta/parsing.hpp"
#include "logging/global.hpp"
#include "hzd/maybe.hpp"
#include "strings/format.hpp"
#include "meta/rules/reduction.hpp"
#include "meta/expr.hpp"
#include "strings/string_map.hpp"
#include "meta/attr_context.hpp"
#include "hzd/ctype.hpp"
#include "meta/module.hpp"
#include "meta/term.hpp"
#include "strings/parsing/parse_int.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/attrs.hpp"

namespace s = j::strings;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunreachable-code"

/*!rules:re2c:common
  re2c:api:style             = free-form;
  re2c:define:YYCTYPE        = "char";
  re2c:define:YYCURSOR       = "state.cursor";
  re2c:define:YYLIMIT        = "state.limit";
  re2c:define:YYMARKER       = "marker";
  re2c:define:YYFILL         = "(void)0;";

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;

  NL         = [\n];
  COMMENT    = "#" [^\n]*;
  WS         = [ ]+;
  MAYBE_WS   = [ ]*;
  NAME       = [A-Z][A-Za-z0-9]*[?]?;
  ID         = [a-z][a-z0-9_-]*;
  LISP_ID    = [^0-9()#\n\x00"'\\@ \t:;][^()#\n\x00"'\\@ \t:;]*;
  ID_OR_NAME = [A-Za-z][A-Za-z0-9_-]*[?]?;
*/

namespace j::meta {
  [[nodiscard]] strings::const_string_view parse_to_eol(base_parser_state & state) noexcept {
    const char * start = state.cursor;
    const char * end = j::strchr(start, '\n');
    if (!end) {
      end = state.limit;
    }
    state.cursor = end;
    return {start, end};
  }

  char skip_ws(base_parser_state & state) noexcept {
    const char * marker;
  loop:
    /*!local:re2c:skip_ws
      !use:common;

      WS | COMMENT { goto loop; }
      NL { state.newline(); goto loop; }
      * { goto out; }
      */
  out:
    return state.cursor[-1];
  }

  static inline s::string format_char(char ch) noexcept {
    switch (ch) {
    case 0: return "'\\0'";
    case '\t': return "'\\t'";
    case '\r': return "'\\r'";
    case '\n': return "'\\n'";
    default: return s::format("'{}'", ch);
    }
  }

  char skip_ws_up_to(base_parser_state & state, const char * J_NOT_NULL mask, const char * J_NOT_NULL ctx) {
    const char ch = skip_ws(state);
    --state.cursor;
    if (!strchr(mask, ch)) {
      s::string msg = s::format("Unexpected {}, expected ", format_char(ch));
      for (i32_t num = 0; *mask; mask++, num++) {
        if (num) {
          msg += mask[1] ? ", " : num == 1 ? " or " : ", or ";
        }
        msg += format_char(*mask);
      }
      state.fail(msg, ctx);
    }
    return ch;
  }

  char skip_ws_past(base_parser_state & state, const char * J_NOT_NULL mask, const char * J_NOT_NULL ctx) {
    char result = skip_ws_up_to(state, mask, ctx);
    ++state.cursor;
    return result;
  }

  char skip_ws_past(base_parser_state & state, const char * J_NOT_NULL mask) {
    char result = skip_ws_up_to(state, mask, "when skipping space");
    ++state.cursor;
    return result;
  }


  [[nodiscard]] attr_type_def maybe_parse_attr_simple_type(base_parser_state & state, s::const_string_view str) noexcept {
    if (str == "Int") {
      return attr_type_def(attr_int);
    } else if (str == "Bool") {
      return attr_type_def(attr_bool);
    } else if (str == "Str") {
      return attr_type_def(attr_str);
    } else if (str == "Tpl") {
      return attr_type_def(attr_tpl);
    } else if (str == "Term") {
      return attr_type_def(attr_term);
    } else if (str == "LispId") {
      return attr_type_def(attr_id);
    } else if (enum_def * ed = (enum_def*)state.mod->node_maybe_at(str, node_enum)) {
      return attr_type_def(ed);
    } else {
      return attr_type_def();
    }
  }


  namespace {
    [[nodiscard]] lbr_t get_break(base_parser_state & state) {
      if (*state.cursor == '-') {
        ++state.cursor;
        return no_break;
      }
      return default_break;
    }

    template<i64_t Mul, u32_t Prefix>
    [[maybe_unused]] inline u64_t read_num(const char * J_NOT_NULL start,
                                           const char * J_NOT_NULL end,
                                           u64_t (*get_num)(char) noexcept) noexcept {
      const bool is_neg = *start == '-';
      if (is_neg || *start == '+') {
        ++start;
      }
      start += Prefix;
      i64_t result = 0;
      do {
        result *= Mul;
        result += get_num(*start);
        if (*++start == '_') {
          ++start;
          continue;
        }
      } while (start < end);
      return is_neg ? -result : result;
    }

    attr_type_def parse_attr_simple_type(base_parser_state & state);

    attr_type_def parse_attr_tuple_type(base_parser_state & state) {
      attr_type_def result(attr_tuple);
      while (true) {
        switch (skip_ws(state)) {
        case '(':
          result.tuple.push_back(parse_attr_tuple_type(state));
          break;
        case ')':
          return static_cast<attr_type_def &&>(result);
        default:
          state.cursor--;
          result.tuple.push_back(parse_attr_simple_type(state));
          break;
        }
      }
    }

    [[nodiscard]] attr_type_def parse_attr_simple_type(base_parser_state & state) {
      attr_type_def result = maybe_parse_attr_simple_type(state, parse_name(state, "when parsing tuple type."));
      state.check((bool)result, "Unknown attr type", "when parsing attr type");
      return result;
    }

    [[nodiscard]] attr_value parse_enum(base_parser_state & state, const enum_def * J_NOT_NULL def) {
      u64_t result = 0U;
    loop:
        result |= def->at(parse_name(state, "when parsing flags value."));
        if (*state.cursor == '|') {
          state.check(def->is_flag_enum, "'|' not in flag enum", "when parsing enum");
          ++state.cursor;
          goto loop;
        }
        return attr_value(def, result);
    }
  }

  [[nodiscard]] i64_t read_dec(const char * J_NOT_NULL start,
                                const char * J_NOT_NULL end) noexcept
  {
    return read_num<10, 0>(start, end, [](char c) noexcept -> u64_t { return c - '0'; });
  }

  [[nodiscard]] i64_t read_hex(const char * J_NOT_NULL start,
                                const char * J_NOT_NULL end) noexcept
  {
    return read_num<16, 2>(start, end, [](char c) noexcept -> u64_t {
      c |= 0x20;
      return c <= '9' ? c - '0' : c - 'a' + 10;
    });
  }

  [[nodiscard]] u64_t read_bin(const char * J_NOT_NULL start,
                                const char * J_NOT_NULL end) noexcept
  {
    return read_num<2, 2>(start, end, [](char c) noexcept -> u64_t { return c - '0'; });
  }

  lbr_t parse_template_body(base_parser_state & state, codegen_template & to, lbr_t next_break);

  lbr_t parse_name_expr_stmt(base_parser_state & state, codegen_template & result, lbr_t lbr, stmt_name_expr_t type, const s::const_string_view ctx) {
    skip_ws_past(state, "[");
    s::const_string_view name = parse_name(state);
    s::const_string_view index_name;
    if (skip_ws_past(state, type == stmt_foreach_v ? ",=" : "=") == ',') {
      index_name = parse_name(state);
      skip_ws_past(state, "=");
    }
    expr * e = parse_expr(state);
    skip_ws_past(state, "]");
    auto & stmt = result.push_back_stmt(type, name, index_name, e);
    stmt.loc = state.loc();
    return parse_template_body(state, *stmt.sub.body, lbr);
  }

  lbr_t parse_indent_stmt(base_parser_state & state, codegen_template & result, lbr_t lbr, tpl_part_type type) {
    char ch = skip_ws(state);
    --state.cursor;
    i8_t indent = j::is_digit(ch) ? parse_int(state) : -1;
    auto & stmt = result.push_back_stmt(type, indent);
    stmt.loc = state.loc();
    parse_template_body(state, *stmt.sub.body, lbr);
    return hard_break;
  }

  J_A(RNN,NODISC) expr * parse_if_expr(base_parser_state & state) {
    skip_ws_past(state, "[");
    expr * e = parse_expr(state);
    skip_ws_past(state, "]");
    return e;
  }

  lbr_t parse_if_stmt_body(base_parser_state & state, tpl_part & stmt, lbr_t lbr) {
    stmt.loc = state.loc();
    lbr_t res = parse_template_body(state, *stmt.sub.body, lbr);
    auto ch = skip_ws(state);
    --state.cursor;
    if (ch != 'E') {
      return res;
    }
    auto n = parse_name(state);
    stmt.sub.ifelse.else_body = ::new codegen_template();
    if (n == "ElseIf") {
      auto & elseif_stmt = stmt.sub.ifelse.else_body->push_back_stmt(stmt_if_v, parse_if_expr(state));
      elseif_stmt.loc = state.loc();
      skip_ws_past(state, "{");
      return parse_if_stmt_body(state, elseif_stmt, get_break(state));
    } else {
      state.check(n == "Else", "Expected Else", "when parsing template");
      skip_ws_past(state, "{");
      return parse_template_body(state, *stmt.sub.ifelse.else_body, get_break(state));
    }
  }

  /// Parse if statement from these markers: `If >>[Expr]<< { ... }`.
  ///
  /// Also used for `ElseIf`.
  lbr_t parse_if_stmt(base_parser_state & state, codegen_template & result, lbr_t lbr) {
    auto & stmt = result.push_back_stmt(stmt_if_v, parse_if_expr(state));
    stmt.loc = state.loc();
    return parse_if_stmt_body(state, stmt, lbr);
  }
  lbr_t parse_with_config_stmt(base_parser_state & state, codegen_template & result, lbr_t, const code_writer_settings & settings) {
    auto & stmt = result.push_back_stmt(settings);
    stmt.loc = state.loc();
    parse_template_body(state, *stmt.sub.body, soft_break);
    return soft_break;
  }
  lbr_t parse_template_stmt(base_parser_state & state, codegen_template & result, lbr_t sub_break) {
    auto n = parse_name(state);
    if (n == "If") {
      return parse_if_stmt(state, result, sub_break);
    } else if (n == "ForEach" || n == "Let" || n == "Set") {
      return parse_name_expr_stmt(state, result, sub_break, n.size() == 7 ? stmt_foreach_v : n[0] == 'L' ? stmt_let_v : stmt_set_v, n);
    } else if (n == "IndentInner" || n == "Indent") {
      return parse_indent_stmt(state, result, sub_break, n.size() == 6 ? tpl_indent : tpl_indent_inner);
    } else if (n == "Comment" || n == "DocComment" || n == "HashComment") {
      return parse_with_config_stmt(state, result, sub_break, n.size() == 7 ? cpp_comment : n.size() == 10 ? cpp_doc_comment : hash_comment);
    }
    state.fail("Unsupported statement", n);
  }

  void parse_template_line(base_parser_state & state, codegen_template & result, lbr_t break_before, lbr_t break_after, bool is_inline = false) {
    if (*state.cursor == ' ') {
      ++state.cursor;
    }
    if (*state.cursor == '\n') {
      ++state.cursor;
      state.newline();
      result.push_back_newline();
      return;
    }
    const char * begin = nullptr;
    const char * end = nullptr;
    const char * marker;
    /*!stags:re2c:tpl_line format = "const char *@@ = nullptr;\n"; */
  loop:
    /*!local:re2c:tpl_line
      !use:common;
      @begin [^%\\\n\x00|]+ {
        result.push_back(strings::const_string_view{begin, state.cursor}, break_before, no_break);
        break_before = no_break;
        goto loop;
      }
      "|" {
        if (is_inline && *state.cursor == ']') {
          result.rtrim(break_after);
          ++state.cursor;
          return;
        }
        result.push_back(strings::const_string_view{state.cursor - 1, state.cursor}, break_before, default_break);
        break_before = default_break;
        goto loop;
      }
      "\\" [^\n] {
        result.push_back(strings::const_string_view{state.cursor - 1, state.cursor}, break_before, default_break);
        break_before = default_break;
        goto loop;
      }
      "%(" {
        result.push_back(parse_expr(state), break_before, default_break).loc = state.loc();
        skip_ws_past(state, ")", "in tpl");
        break_before = default_break;
        goto loop;
      }
      NL {
        if (result.parts) {
          result.parts.back().break_after = break_after;
        }
        state.newline();
        return;
      }
      * { goto fail; }
     */
  fail:
    state.fail("Unexpected token", "when parsing template line.");
  }

  strings::const_string_view parse_name_or_id(base_parser_state & state, strings::const_string_view ctx, bool allow_id, bool allow_name) {
    skip_ws(state);
    const char *marker, *begin = --state.cursor;
    /*!local:re2c:name
      !use:common;
      [@]? ID_OR_NAME {
        state.check((*begin >= 'a') ? allow_id : allow_name, (*begin >= 'a') ? "Did not expect lower-cased name" : "Did not expect an upper-cased name", ctx);
        return {begin, state.cursor};
      }
      * { goto fail; }
     */
  fail:
    state.fail("Unexpected token", ctx);
  }

  strings::const_string_view parse_name(base_parser_state & state, strings::const_string_view ctx) {
    return parse_name_or_id(state, ctx, false, true);
  }
  strings::const_string_view parse_id(base_parser_state & state, strings::const_string_view ctx) {
    return parse_name_or_id(state, ctx, true, false);
  }

  codegen_template parse_template_out(base_parser_state & state) {
    skip_ws_past(state, "[");
    return parse_template(state);
  }

  lbr_t parse_template_body(base_parser_state & state, codegen_template & to, lbr_t next_break) {
    while (true) {
      switch(skip_ws(state)) {
      case '|':
        parse_template_line(state, to, next_break, soft_break);
        next_break = default_break;
        break;
      case '{': {
        next_break = parse_template_stmt(state, to, get_break(state));
        break;
      }
      case '-':
        state.check(*state.cursor == '}' || *state.cursor == ']', "Unexpected -", "when parsing template.");
        state.cursor++;
        if (to.parts) {
          to.parts.back().break_after = no_break;
          return default_break;
        }
        return no_break;
      case '}': case ']':
        return next_break;
      default:
        state.fail("Unexpected token", "when parsing template.");
      }
    }
  }

  codegen_template parse_template(base_parser_state & state) {
    codegen_template result;
    if (state.cursor[0] == '|') {
      ++state.cursor;
      parse_template_line(state, result, default_break, default_break, true);
    } else {
      codegen_template * stack[32]{&result};
      u32_t stmt_depth = 0U;
      parse_template_body(state, result, get_break(state));
    }
    return static_cast<codegen_template &&>(result);
  }

  attr_type_def parse_attr_type(base_parser_state & state) {
    if (skip_ws(state) == '(') {
      return parse_attr_tuple_type(state);
    } else {
      --state.cursor;
      return parse_attr_simple_type(state);
    }
  }

  attr_value parse_tuple(base_parser_state & state, attr_type_def * type) {
    if (!type) {
      return parse_tuple(state);
    }
    vector<attr_value> values;
    for (auto & t : type->tuple) {
      state.check(skip_ws(state) != ')', "Premature end of tuple", "when parsing tuple");
      --state.cursor;
      values.push_back(parse_attr_value(state, &t));
    }
    skip_ws_past(state, ")");
    return attr_value(span<attr_value>(values));
  }

  attr_value parse_tuple(base_parser_state & state) {
    vector<attr_value> values;
    while (true) {
      if (skip_ws(state) == ')') {
        return attr_value(span<attr_value>(values));
      }
      --state.cursor;
      values.push_back(parse_attr_value(state));
    }
  }

  attr_value parse_map(base_parser_state & state) {
    attr_map_t map;
    char ch = skip_ws(state);
    if (ch != '}') {
      --state.cursor;
    }
    while (true) {
      if (ch == '}') {
        return attr_value(static_cast<attr_map_t &&>(map));
      }
      s::const_string_view key = parse_name_or_id(state, "when parsing map key");
      skip_ws_past(state, ":");
      map.emplace(key, parse_attr_value(state));
      ch = skip_ws_past(state, ",}");
    }
  }

  [[nodiscard]] attr_value maybe_parse_tuple_out(base_parser_state & state) {
    if (skip_ws(state) == '(') {
      return parse_tuple(state);
    }
    return {};
  }

  [[nodiscard]] attr_value parse_tuple_out(base_parser_state & state) {
    skip_ws_past(state, "(");
    return parse_tuple(state);
  }

  [[nodiscard]] strings::string parse_string_out(base_parser_state & state) {
    skip_ws_past(state, "\"");
    return parse_string(state);
  }

  [[nodiscard]] strings::string parse_string(base_parser_state & state) {
    strings::string result;
    const char * begin = nullptr;
    const char * marker;
    /*!stags:re2c:str format = "const char *@@ = nullptr;\n"; */
  loop:
    /*!local:re2c:str
      !use:common;
      "\\" [^\n] {
        result.push_back(state.cursor[-1] == 'n' ? '\n' : state.cursor[-1]);
        goto loop;
      }
      @begin [^"\\\n\x00]+ {
        result.append(begin, state.cursor);
        goto loop;
      }
      ["] { return result; }
      * { goto fail; }
     */
  fail:
    state.fail("Unexpected token", "when parsing string.");
  }

  J_A(NODISC) lisp::id_name parse_lisp_id(base_parser_state & state) {
    const char * begin = state.cursor;
    const char * mid = nullptr;
    const char * marker;
    strings::const_string_view pkg = state.default_package;
    /*!stags:re2c:lisp_id format = "const char *@@ = nullptr;\n"; */
    /*!local:re2c:lisp_id
      !use:common;
      LISP_ID @mid ":" LISP_ID {
        pkg = strings::const_string_view(begin, mid);
        begin = mid + 1;
        goto out;
      }
      LISP_ID {
        goto out;
      }
      * { goto unexpected; }
     */
  out:
    state.check((bool)pkg, "No default package");
    return {pkg, strings::string(begin, state.cursor)};
  unexpected:
    state.fail("Unexpected token", "when parsing lisp id.");
  }

  basic_node_set parse_term_set(base_parser_state & state) {
    basic_node_set terms;
    bool negated = false;
    if (skip_ws(state) == '!') {
      negated = true;
    } else {
      state.cursor--;
    }
    do {
      terms.add(*state.term_at(parse_id(state, "when parsing pattern")));
    } while (skip_ws(state) == '|');
    state.cursor--;
    return negated ? basic_node_set::all(state.mod, node_term) - terms : terms;
  }

  [[nodiscard]] maybe<i64_t> maybe_parse_int(base_parser_state & state) noexcept {
    skip_ws(state);
    const char * begin = --state.cursor;
    const char * marker;
    /*!local:re2c:parse_int
      !use:common;
      re2c:flags:tags = 0;

      "0b" [01]+ {
        return maybe<i64_t>(read_bin(begin, state.cursor));
      }
      "0x" [0-9a-fA-F]+ {
        return maybe<i64_t>(read_hex(begin, state.cursor));
      }
      [-]? [0-9]+ {
        return maybe<i64_t>(read_dec(begin, state.cursor));
      }
      * { --state.cursor; return {}; }
    */
  }

  [[nodiscard]] i64_t parse_int(base_parser_state & state) {
    return maybe_parse_int(state).value();
  }

  attr_value parse_attr_value(base_parser_state & state, attr_type_def * type, bool is_required) {
    if (type && type->type == attr_enum) {
      return parse_enum(state, type->enum_def);
    }
    if (type && type->type == attr_type_none) {
      type = nullptr;
    }

    /*!stags:re2c:val format = "const char *@@ = nullptr;\n"; */
    const char * begin = nullptr;
    const char * s2 = nullptr;
    const char * marker;
    attr_value result;
  loop:
    /*!local:re2c:val
      !use:common;
      NL {
        state.newline();
        goto out;
      }
      WS { goto loop; }
      ";" | COMMENT | "None" { goto out; }
      ["] {
        result = attr_value(parse_string(state));
        goto out;
      }
      "'" {
        result = attr_value(parse_lisp_id(state));
        goto out;
      }
      "[" {
        result = attr_value(parse_template(state));
        goto out;
      }
      "{" {
        result = parse_map(state);
        goto out;
      }
      "(" {
        result = parse_tuple(state, type);
        goto out;
      }
      [@)},] {
        --state.cursor;
        goto out;
      }
      ("True"|"False") {
        result = attr_value(state.cursor[-2] == 'u');
        goto out;
      }
      @begin NAME "." @s2 NAME {
        enum_def * ed = (enum_def*)state.mod->node_maybe_at(s::const_string_view(begin, s2 - 1), node_enum);
        state.check(ed, "Unknown enum", s::const_string_view(begin, s2 - 1));
        result = ed->value(s::const_string_view(s2, state.cursor));
        goto out;
      }
      @begin ID_OR_NAME {
        node * n = state.mod->node_maybe_at(s::const_string_view(begin, state.cursor));
        state.check(n, "Unknown node", s::const_string_view(begin, state.cursor));
        result = attr_value(n);
        goto out;
      }
      * {
        --state.cursor;
        if (maybe<i64_t> num = maybe_parse_int(state)) {
          result = attr_value(num.value_unsafe());
        }
        goto out;
      }
     */
  out:
    state.check((!is_required && !result) || !type || type->type == result.type, "Unexpected constant type", "when parsing attr value");
    return result;
  fail:
    state.fail("Unexpected token", "when parsing value.");
  }

  [[nodiscard]] operator_type parse_te_operator(base_parser_state & state) {
    skip_ws(state);
    const char * begin = --state.cursor;
    const char * marker;
    /*!local:re2c:operator
      !use:common;
      "$+!" { return operator_type::append_fx; }
      "!+$" { return operator_type::prepend_fx; }
      "+=!" { return operator_type::add_fx; }
      "+=$" { return operator_type::add_value; }
      "||" { return operator_type::lor; }
      "&&" { return operator_type::land; }
      "^^" { return operator_type::lxor; }
      "==" { return operator_type::eq; }
      "!=" { return operator_type::neq; }
      "<=" { return operator_type::le; }
      "<"  { return operator_type::lt; }
      ">=" { return operator_type::ge; }
      ">"  { return operator_type::gt; }
      "-=" { return operator_type::assign_sub; }
      "+=" { return operator_type::assign_add; }
      "*=" { return operator_type::assign_mul; }
      "/=" { return operator_type::assign_div; }
      "[]" { return operator_type::index; }
      "="  { return operator_type::assign; }
      "-"  { return operator_type::sub; }
      "+"  { return operator_type::add; }
      "*"  { return operator_type::mul; }
      "/"  { return operator_type::div; }
      * { goto fail; }
    */
  fail:
    state.fail("Unknown operator", strings::const_string_view(begin, 3));
  }
}
#pragma clang diagnostic pop
