#include "cpp_codegen.hpp"
#include "meta/attr_value_iteration.hpp"
#include "meta/expr.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "meta/expr_scope.hpp"
#include "exceptions/assert.hpp"
#include "meta/term.hpp"
#include "meta/attr_value.hpp"
#include "strings/format.hpp"
#include "containers/pair.hpp"

namespace j::meta {
  namespace s = strings;

  namespace {
    void copy_tpl_part(tpl_part * to, const tpl_part & from) {
      j::memcpy(to, &from, sizeof(tpl_part));
      switch (from.type) {
      case tpl_part_none:
      case tpl_newline:
        break;
      case tpl_expr:
        to->subst_expr = ::new expr(*from.subst_expr);
        break;
      case tpl_verbatim:
        ::new (&to->content) strings::string(from.content);
        break;
      case tpl_if:
        to->sub.body = from.sub.body ? ::new codegen_template(*from.sub.body) : nullptr;
        to->sub.ifelse.e = ::new expr(*from.sub.ifelse.e);
        to->sub.ifelse.else_body = from.sub.ifelse.else_body ? ::new codegen_template(*from.sub.ifelse.else_body) : nullptr;
        break;
      case tpl_foreach:
        ::new (&to->sub.let.index_name) s::string(from.sub.let.index_name);
        [[fallthrough]];
      case tpl_let:
        to->sub.body = ::new codegen_template(*from.sub.body);
        [[fallthrough]];
      case tpl_set:
        to->sub.let.e = ::new expr(*from.sub.let.e);
        ::new (&to->sub.let.name) s::string(from.sub.let.name);
        break;
      case tpl_with_writer_settings:
        to->sub.body = ::new codegen_template(*from.sub.body);
        to->sub.writer_settings = from.sub.writer_settings;
        break;
      case tpl_indent:
      case tpl_indent_inner:
        to->sub.body = ::new codegen_template(*from.sub.body);
        to->sub.indent = from.sub.indent;
        break;
      }
    }
  }

  tpl_part & tpl_part::operator=(tpl_part && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      j::memcpy(this, &rhs, sizeof(tpl_part));
      rhs.type = tpl_part_none;
    }
    return *this;
  }

  tpl_part::tpl_part(const tpl_part & rhs) {
    copy_tpl_part(this, rhs);
  }

  tpl_part & tpl_part::operator=(const tpl_part & rhs) {
    if (J_LIKELY(this != &rhs)) {
      clear();
      copy_tpl_part(this, rhs);
    }
    return *this;
  }

  void tpl_part::clear() noexcept {
    switch (type) {
    case tpl_part_none:
    case tpl_newline:
    case tpl_indent:
    case tpl_indent_inner:
    case tpl_with_writer_settings:
      break;
    case tpl_expr:
      ::delete subst_expr;
      subst_expr = nullptr;
      break;
    case tpl_verbatim:
      content.~string();
      break;
    case tpl_if:
      ::delete sub.body;
      ::delete sub.ifelse.e;
      ::delete sub.ifelse.else_body;
      sub.body = sub.ifelse.else_body = nullptr;
      sub.ifelse.e = nullptr;
    break;
    case tpl_foreach:
      sub.let.index_name.~string();
      [[fallthrough]];
    case tpl_let:
      ::delete sub.body;
      [[fallthrough]];
    case tpl_set:
      ::delete sub.let.e;
      sub.let.name.~string();
      break;
    }
    type = tpl_part_none;
  }

  tpl_part::~tpl_part() {
    clear();
  }

  J_A(FLATTEN,NODISC) u32_t tpl_part_hash::operator()(const tpl_part & part) const noexcept {
    u32_t h = 1U + part.type + ((u32_t)part.break_before << 10) + ((u32_t)part.break_after << 15);
    switch (part.type) {
    case tpl_part_none:
    case tpl_newline:
      break;
    case tpl_expr:
      h ^= expr_hash{}(*part.subst_expr);
      break;
    case tpl_verbatim:
      h += strings::hash_string(part.content);
      break;
    case tpl_with_writer_settings:
      h += tpl_hash{}.operator()(*part.sub.body)
        ^ (part.sub.writer_settings.inner_indent)
        ^ (part.sub.writer_settings.indent << 8U)
        ^ (part.sub.writer_settings.begin_token ? strings::hash_string(part.sub.writer_settings.begin_token) : 11911)
        ^ (part.sub.writer_settings.pack_empty_lines.value() << 16);
      break;
    case tpl_indent:
    case tpl_indent_inner:
      h += tpl_hash{}.operator()(*part.sub.body) ^ part.sub.indent << 8;
      break;
    case tpl_if:
      h += tpl_hash{}.operator()(*part.sub.body) ^ expr_hash{}.operator()(*part.sub.ifelse.e);
      if (part.sub.ifelse.else_body) {
        h -= tpl_hash{}.operator()(*part.sub.ifelse.else_body);
      }
      break;
    case tpl_foreach:
      h -= strings::hash_string(part.sub.let.index_name);
      [[fallthrough]];
    case tpl_let:
      h += tpl_hash{}.operator()(*part.sub.body);
      [[fallthrough]];
    case tpl_set:
      h -= expr_hash{}.operator()(*part.sub.let.e) ^ strings::hash_string(part.sub.let.name);
      break;
    }
    return crc32((u32_t)part.type, h);
  }

  tpl_part::tpl_part(strings::const_string_view text, lbr_t break_before, lbr_t break_after) noexcept
    : type(tpl_verbatim),
      break_before(break_before),
      break_after(break_after),
      content{text}
  {
  }

  tpl_part::tpl_part(expr * J_NOT_NULL e, lbr_t break_before, lbr_t break_after) noexcept
    : type(tpl_expr),
      break_before(break_before),
      break_after(break_after),
      subst_expr(e)
  { }

  tpl_part::tpl_part(tpl_part_type type) noexcept
    : type(type)
  {
  }

  tpl_part::tpl_part(tpl_part_type type, codegen_template && tpl,i8_t indent) noexcept
    : type(type)
  {
    sub.body = ::new codegen_template(static_cast<codegen_template &&>(tpl));
    sub.indent = indent;
  }

  tpl_part::tpl_part(codegen_template && tpl, const code_writer_settings & settings) noexcept
    : type(tpl_with_writer_settings)
  {
    sub.body = ::new codegen_template(static_cast<codegen_template &&>(tpl));
    sub.writer_settings = settings;
  }

  tpl_part::tpl_part(stmt_name_expr_t type, strings::string && name, expr * J_NOT_NULL e) noexcept
    : type((tpl_part_type)type)
  {
    sub.body = type != stmt_set_v ? ::new codegen_template() : nullptr;
    ::new (&sub.let.name) strings::string(static_cast<strings::string &&>(name));
    sub.let.e = e;
  }

  tpl_part::tpl_part(stmt_name_expr_t type, strings::string && name, strings::string && index_name, expr * J_NOT_NULL e) noexcept
    : type((tpl_part_type)type)
  {
    sub.body = type != stmt_set_v ? ::new codegen_template() : nullptr;
    ::new (&sub.let.name) strings::string(static_cast<strings::string &&>(name));
    ::new (&sub.let.index_name) strings::string(static_cast<strings::string &&>(index_name));
    sub.let.e = e;
  }

  tpl_part::tpl_part(stmt_if_t, expr * J_NOT_NULL e) noexcept
    : type(tpl_if)
  {
    sub.body = ::new codegen_template();
    sub.ifelse.e = e;
    sub.ifelse.else_body = nullptr;
  }

  [[nodiscard]] bool tpl_part::operator==(const tpl_part & rhs) const noexcept {
    if (type != rhs.type || break_before != rhs.break_before || break_after != rhs.break_after) {
      return false;
    }
    switch (type) {
    case tpl_part_none:
    case tpl_newline:
      return true;
    case tpl_expr:
      return *subst_expr == *rhs.subst_expr;
    case tpl_verbatim:
      return content == rhs.content ;
    case tpl_if:
      return *sub.body == *rhs.sub.body && *sub.ifelse.e == *rhs.sub.ifelse.e &&
        (sub.ifelse.else_body ? rhs.sub.ifelse.else_body && *sub.ifelse.else_body == *rhs.sub.ifelse.else_body : !rhs.sub.ifelse.else_body);
    case tpl_foreach:
      if (sub.let.index_name != rhs.sub.let.index_name) {
        return false;
      }
      [[fallthrough]];
    case tpl_let:
      if (*sub.body != *rhs.sub.body) {
        return false;
      }
      [[fallthrough]];
    case tpl_set:
      return *sub.let.e == *rhs.sub.let.e && sub.let.name == rhs.sub.let.name;
    case tpl_with_writer_settings:
      return *sub.body == *rhs.sub.body && sub.writer_settings == rhs.sub.writer_settings;
    case tpl_indent:
    case tpl_indent_inner:
      return *sub.body == *rhs.sub.body && sub.indent == rhs.sub.indent;
    }
  }

  [[nodiscard]] bool tpl_part::operator<(const tpl_part & rhs) const noexcept {
    if (type != rhs.type) {
      return type < rhs.type;
    }
    if (break_before != rhs.break_before) {
      return break_before < rhs.break_before;
    }
    if (break_after != rhs.break_after) {
      return break_after < rhs.break_after;
    }
    switch (type) {
    case tpl_part_none:
    case tpl_newline:
      return false;
    case tpl_expr:
      return *subst_expr < *rhs.subst_expr;
    case tpl_verbatim:
      return content < rhs.content;
    case tpl_if:
      return *sub.body < *rhs.sub.body && *sub.ifelse.e < *rhs.sub.ifelse.e &&
        (sub.ifelse.else_body ? rhs.sub.ifelse.else_body && *sub.ifelse.else_body < *rhs.sub.ifelse.else_body : (bool)rhs.sub.ifelse.else_body);
    case tpl_foreach:
      if (sub.let.index_name != rhs.sub.let.index_name) {
        return sub.let.index_name < rhs.sub.let.index_name;
      }
      [[fallthrough]];
    case tpl_let:
      if (*sub.body != *rhs.sub.body) {
        return *sub.body < *rhs.sub.body;
      }
      [[fallthrough]];
    case tpl_set:
      if (*sub.let.e != *rhs.sub.let.e) {
        return *sub.let.e < *rhs.sub.let.e;
      }
      return sub.let.name < rhs.sub.let.name;
    case tpl_with_writer_settings:
      if (*sub.body != *rhs.sub.body) {
        return *sub.body < *rhs.sub.body;
      }
      return sub.writer_settings < rhs.sub.writer_settings;
    case tpl_indent:
    case tpl_indent_inner:
      if (*sub.body != *rhs.sub.body) {
        return *sub.body < *rhs.sub.body;
      }
      return sub.indent < rhs.sub.indent;
    }
  }

  codegen_template::~codegen_template() { }

  codegen_template::codegen_template(const codegen_template & rhs) noexcept
    : parts(rhs.parts)
  { }

  codegen_template::codegen_template(strings::const_string_view str, lbr_t break_before, lbr_t break_after) noexcept {
    push_back(str, break_before, break_after);
  }

  codegen_template::codegen_template(strings::string && str, lbr_t break_before, lbr_t break_after) noexcept {
    push_back(static_cast<strings::string &&>(str), break_before, break_after);
  }

  codegen_template & codegen_template::operator=(const codegen_template & rhs) noexcept {
    parts = rhs.parts;
    return *this;
  }

  [[nodiscard]] bool codegen_template::operator==(const codegen_template & rhs) const noexcept {
    if (parts.size() != rhs.parts.size()) {
      return false;
    }
    for (auto it1 = parts.begin(), it2 = rhs.parts.begin(), end1 = parts.end();
         it1 != end1; ++it1, ++it2) {
      if (*it1 != *it2) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool codegen_template::operator<(const codegen_template & rhs) const noexcept {
    if (parts.size() != rhs.parts.size()) {
      return parts.size() < rhs.parts.size();
    }
    for (auto it1 = parts.begin(), it2 = rhs.parts.begin(), end1 = parts.end();
         it1 != end1; ++it1, ++it2) {
      if (*it1 != *it2) {
        return *it1 < *it2;
      }
    }
    return false;
  }
  tpl_part & codegen_template::push_back(expr * J_NOT_NULL e, lbr_t break_before, lbr_t break_after) {
    return parts.emplace_back(e, break_before, break_after);
  }
  void codegen_template::push_back(strings::const_string_view text, lbr_t break_before, lbr_t break_after) {
    if (parts && parts.back().type == tpl_verbatim) {
      auto & back = parts.back();
      if (text) {
        if ((back.break_after >= soft_break && break_before != no_break) || break_before >= soft_break) {
          back.content.push_back('\n');
        }
        back.content += text;
        back.break_after = break_after;
      } else {
        const lbr_t max_break = J_MAX(break_before, break_after);
        if (break_before == no_break || (back.break_after < max_break)) {
          back.break_after = max_break;
        }
      }
      return;
    }
    parts.emplace_back(text, break_before, break_after);
  }
  void codegen_template::push_back_newline() {
    parts.emplace_back(tpl_newline);
  }

  tpl_part & codegen_template::push_back(const tpl_part & part) {
    return parts.push_back(part);
  }
  tpl_part & codegen_template::push_back_stmt(const code_writer_settings & settings) {
    return parts.emplace_back(codegen_template(), settings);
  }
  tpl_part & codegen_template::push_back_stmt(tpl_part_type type, i8_t indent) {
    return parts.emplace_back(type, codegen_template(), indent);
  }
  tpl_part & codegen_template::push_back_stmt(stmt_name_expr_t tag, strings::const_string_view name, expr * J_NOT_NULL e) noexcept {
    return parts.emplace_back(tag, name, e);
  }
  tpl_part & codegen_template::push_back_stmt(stmt_name_expr_t tag, strings::string && name, expr * J_NOT_NULL e) noexcept {
    return parts.emplace_back(tag, static_cast<s::string &&>(name), e);
  }
  tpl_part & codegen_template::push_back_stmt(stmt_name_expr_t tag, strings::string && name, strings::string && index_name, expr * J_NOT_NULL e) noexcept {
    return parts.emplace_back(tag, static_cast<s::string &&>(name), static_cast<s::string &&>(index_name), e);
  }

  tpl_part & codegen_template::push_back_stmt(stmt_name_expr_t tag, s::const_string_view name, s::const_string_view index_name, expr * J_NOT_NULL e) noexcept {
    return parts.emplace_back(tag, name, index_name, e);
  }

  tpl_part & codegen_template::push_back_stmt(stmt_if_t, expr * J_NOT_NULL e) noexcept {
    return parts.emplace_back(stmt_if_v, e);
  }

  void codegen_template::expand(code_writer & to, expr_scope & ctx) const {
    for (auto & part : *this) {
      J_SCOPE_INFO(source_loc(part.loc));
      auto g = ctx.enter_tpl(&part);
      switch (part.break_before) {
      case lbr_t::normal: break;
      case lbr_t::none: to.clear_break(); break;
      case lbr_t::hard: to.newline(); break;
      case lbr_t::soft: to.begin_line(); break;
      }
      switch (part.type) {
      case tpl_part_none: J_UNREACHABLE();
      case tpl_verbatim: to.write_with_line_breaks(part.content); break;
      case tpl_newline: to.queue_empty_line(); break;
      case tpl_expr: expand_value(to, ctx, eval_expr(ctx, part.subst_expr)); break;
      case tpl_let: {
        expr_scope ch{&ctx};
        ch[part.sub.let.name] = eval_expr(ctx, part.sub.let.e);
        part.sub.body->expand(to, ch);
        break;
      }
      case tpl_set:
        ctx.assign(part.sub.let.name, eval_expr(ctx, part.sub.let.e));
        break;
      case tpl_foreach: {
        attr_value val = eval_expr(ctx, part.sub.let.e);
        expr_scope ch{&ctx};
        u32_t i = 0U;
        for (attr_value it : iterate(&val, ctx.root->mod)) {
          ch[part.sub.let.name ? part.sub.let.name : "It"] = static_cast<attr_value &&>(it);
          if (part.sub.let.index_name) {
            ch[part.sub.let.index_name] = attr_value(i++);
          }
          part.sub.body->expand(to, ch);
        }
        break;
      }
      case tpl_if:
        if (eval_expr(ctx, part.sub.ifelse.e).value_empty()) {
          if (part.sub.ifelse.else_body) {
            part.sub.ifelse.else_body->expand(to, ctx);
          }
        } else if (part.sub.body) {
          part.sub.body->expand(to, ctx);
        }
        break;
      case tpl_with_writer_settings: {
        to.flush_empty_line();
        to.to->flush();
        {
          auto g = to.enter(part.sub.writer_settings);
          part.sub.body->expand(to, ctx);
        }
        to.queue_break();
        break;
      }
      case tpl_indent:
      case tpl_indent_inner: {
        {
          auto g = to.descend_guarded(part.sub.indent);
          part.sub.body->expand(to, ctx);
        }
        to.clear_empty_line();
        break;
      }
      }

      switch (part.break_after) {
      case lbr_t::normal: break;
      case lbr_t::none: to.clear_break(); break;
      case lbr_t::hard: to.newline(); break;
      case lbr_t::soft: to.queue_break(); break;
      }
    }
  }
  void codegen_template::expand_value(code_writer & to, expr_scope & ctx, const attr_value & value) const {
    switch (value.type) {
    case attr_bool: to.write(value.as_bool() ? "true" : "false"); break;
    case attr_str: to.write_with_line_breaks(value.as_str()); break;
    case attr_int: to.write_formatted("{}", value.as_int()); break;
    case attr_tpl: value.as_tpl().expand(to, ctx); break;
    default:
      ctx.throw_expr_error(s::format("Unsupported attribute value in template {}", value.type));
    }
  }

  void codegen_template::rtrim(lbr_t br) noexcept {
    if (parts && parts.back().type == tpl_verbatim) {
      const char * begin = parts.back().content.begin();
      const char * end = parts.back().content.end();
      while (end != begin && end[-1] == ' ') {
        --end;
      }
      if (begin == end) {
        parts.pop_back();
      } else if (end != parts.back().content.end()) {
        parts.back().content = strings::string(begin, end);
      }
    }
    if (parts) {
      parts.back().break_after = br;
    }
  }
}
