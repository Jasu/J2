#include <strings/parsing/parse_int.hpp>
#include <strings/unicode/utf8.hpp>
#include <logging/global.hpp>
#include <util/hex.hpp>
#include <hzd/mem.hpp>
#include <parsing/precedence_parser.hpp>
#include <mem/unique_ptr.hpp>
#include <hzd/mem.hpp>
#include <strings/regex/ast.hpp>
#include <strings/unicode/utf8.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic ignored "-Wunused"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-variable"

/*!types:re2c*/
/*!maxnmatch:re2c*/
/*!rules:re2c

  re2c:define:YYCURSOR = pos.cursor;
  re2c:define:YYMARKER = pos.marker;
  re2c:define:YYCTYPE  = u8_t;

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;

  // The buffer to be parsed is the whole input + 0-terminator.
  // (0-terminator is implied by the lack of re2c:eof)
  re2c:yyfill:enable = 0;

  ESC = "\\";
  FORBIDDEN = [\r\n\x00];
  NAMED_QUANTIFIER = [?*+];
  QUANTIFIER_START = NAMED_QUANTIFIER | [{];
  NOT_QUANTIFIER_START = [^?*+{];
  RE_SPECIAL = QUANTIFIER_START | [$^] | [.\[] | [|()] | ESC;
  RE_NORMAL = . \ (FORBIDDEN | RE_SPECIAL);
  NAMED_CHAR_CLASS = [dsw];
  SPECIAL_ESCAPE = [rnt];
  NORMAL_ESCAPE = . \ (FORBIDDEN | NAMED_CHAR_CLASS | SPECIAL_ESCAPE);
*/

namespace p = j::parsing;
namespace j::strings::regex {
  static inline constexpr ascii_mask char_class_d{{0x3ff000000000000UL, 0x0000000000000000UL}};
  static inline constexpr ascii_mask char_class_w{{0x3ff000000000000UL, 0x07fffffe87fffffeUL}};
  static inline constexpr ascii_mask char_class_s{{0x000000100003e00UL, 0x0000000000000000UL}};

  J_A(ND,NODISC) static const ascii_mask & named_char_class_mask(char name) {
    switch(name) {
    case 'd': return char_class_d;
    case 'w': return char_class_w;
    case 's': return char_class_s;
    default: J_FAIL("Unsupported char chass {}", name);
    }
  }

  struct J_AT(HIDDEN) reader_pos final {
    const u8_t *cursor;
    const u8_t *marker;
  };

  static node_char_class * read_char_class(reader_pos & pos) {
    char_class cls{};
    const u8_t *a = nullptr;
    const bool is_negated = *pos.cursor == '^';
    if (is_negated) {
      ++pos.cursor;
    }
    const_string_view ch;
    bool is_range = false;
    /*!stags:re2c format = 'const u8_t *@@ = nullptr;'; */
    for (;;) {
      /*!use:re2c
        NORM = [^\r\n\x00\]\\];

        "]" { goto out; }

        ESC NAMED_CHAR_CLASS {
          cls.ascii_mask |= named_char_class_mask(pos.cursor[-1]);
          continue;
        }

        "-" {
          if (!ch) {
            ch = "-";
          } else if (is_range) {
            cls.add_range(ch, "-");
            is_range = false;
            ch = {};
          } else {
            is_range = true;
          }
          continue;
        }

        @a (ESC NORMAL_ESCAPE | NORM) {
          if (*a == '\\') {
            ++a;
          }
          if (is_range) {
            cls.add_range(ch, {(const char*)a, (const char*)pos.cursor});
            is_range = false;
            ch = {};
          } else {
            if (ch) {
              cls.add_char(ch);
            }
            ch = {(const char*)a, (const char*)pos.cursor};
          }
          continue;
        }

        [\x00] { goto fail; }
        * { goto fail; }
      */
    }

  fail:
    J_THROW("Expected a close bracket");
  out:
    if (ch) {
      cls.add_char(ch);
      if (is_range) {
        cls.add_char("-");
      }
    }
    return ::new node_char_class(static_cast<char_class &&>(cls), is_negated);
  }

#define J_AST_STACK_SZ 32
  J_A(RNN,NODISC) static node_literal * alloc_literal(const u8_t * begin, u32_t sz) noexcept {
    char *data = (char*)j::allocate_zero(sizeof(node_literal) + sz + 1);
    auto lit = ::new (data) node_literal(sz);
    j::memcpy(lit->char_array, begin, sz);
    data[sizeof(node_literal) + sz] = '\0';
    return lit;
  }

  enum J_AT(HIDDEN) regex_expr_type {
    expr_epsilon,
    expr_single,
    expr_cat,
    expr_alt,
  };

  struct J_AT(HIDDEN) regex_expr final {
    regex_expr_type type = expr_epsilon;

    union {
      node_base *node = nullptr;
      alternate *alt;
    } first;
    union {
      node_base *node = nullptr;
      alternate **alt;
    } next;

    node_base *to_node() {
      switch (type) {
      case expr_epsilon:
        return alloc_literal(nullptr, 0U);
      case expr_single:
      case expr_cat:
        return first.node;
      case expr_alt:
        return ::new node_alternation(first.alt);
      }
    }

    J_A(AI,ND) inline regex_expr() noexcept = default;

    J_A(AI,ND) inline regex_expr(node_base * J_AA(NN) n)
      : type(expr_single),
        first{n}
    { }

    J_A(AI,ND) inline regex_expr(alternate *first, alternate**next)
      : type(expr_alt),
        first{.alt = first},
        next{.alt = next}
    { }

    J_A(AI,ND) inline regex_expr(node_base * J_AA(NN) lhs, node_base * J_AA(NN) rhs)
      : type(expr_cat),
        first{lhs},
        next{lhs}
    {
      lhs->next = rhs;
    }
  };

  enum J_AT(HIDDEN) regex_op_type : u8_t {
    op_alt,
    op_cat,
    op_rep,
  };

  static constinit const p::operator_info operator_info[]{
    [op_alt] = { 1, p::infix, "|" },
    [op_cat] = { 10, p::infix, "" },
    [op_rep] = { 20, p::postfix, "*" },
  };

  struct J_AT(HIDDEN) regex_op final {
    regex_op_type type;
    u32_t min = 0;
    u32_t max = 0;
  };

  J_A(RNN,AI,NODISC) static inline const p::operator_info * get_op_info(const regex_op & op) noexcept {
    return &operator_info[op.type];
  }

  static void dump_expr(const regex_expr & e, const char * name) {
    switch (e.type) {
    case expr_epsilon:
      J_DEBUG("{} is epsilon", name ? name : "Expr");
      break;
    case expr_single:
      J_DEBUG("{} is single", name ? name : "Expr");
      dump_single(e.first.node, 2);
      break;
    case expr_cat:
      J_DEBUG("{} is cat", name ? name : "Expr");
      break;
    case expr_alt:
      J_DEBUG("{} is alt", name ? name : "Expr");
      break;
    default:
      J_DEBUG("{} is invalid {:08X}", name ? name : "Expr", (u32_t)e.type);
      break;
    }
  }

  J_A(NODISC) static inline regex_expr reduce_binary(const regex_op & op, regex_expr && lhs, regex_expr && rhs) noexcept {
    switch (op.type) {
    case op_rep:
      J_FAIL("Reduced unary op as bin");
    case op_alt:
      // J_DEBUG("Reducing alternation");
      if (lhs.type == expr_alt) {
        if (rhs.type == expr_alt) {
          *lhs.next.alt = rhs.first.alt;
          lhs.next.alt = rhs.next.alt;
        } else {
          alternate * rhs_alt = ::new alternate{rhs.to_node(), nullptr};
          *lhs.next.alt = rhs_alt;
          lhs.next.alt = &rhs_alt->next;
        }
        return lhs;
      } else {
        alternate * lhs_alt = ::new alternate{lhs.to_node(), nullptr};
        if (rhs.type == expr_alt) {
          lhs_alt->next = rhs.first.alt;
          rhs.first.alt = lhs_alt;
          return rhs;
        }
        alternate * rhs_alt = ::new alternate{rhs.to_node(), nullptr};
        lhs_alt->next = rhs_alt;
        return {lhs_alt, &rhs_alt->next};
      }
    case op_cat:
      if (lhs.type == expr_epsilon) {
        return rhs;
      } else if (rhs.type == expr_epsilon) {
        return lhs;
      }

      regex_expr result = (lhs.type == expr_cat || lhs.type == expr_single) ? lhs : regex_expr(lhs.to_node());
      result.type = expr_cat;
      node_base *parent_node = result.next.node;
      node_base *prev_node = parent_node ? parent_node->next : result.first.node;
      for (node_base *next_node = (rhs.type == expr_cat || rhs.type == expr_single) ? rhs.first.node : rhs.to_node(); next_node; next_node = next_node->next) {
        if (prev_node->type == nt_literal && next_node->type == nt_literal) {
          auto next_lit = (node_literal*)next_node;
          auto prev_lit = (node_literal*)prev_node;
          prev_lit = (node_literal*)j::reallocate(prev_lit, sizeof(node_literal) + next_lit->len + prev_lit->len + 1);
          j::memcpy(prev_lit->char_array + prev_lit->len, next_lit->char_array, next_lit->len + 1);
          prev_lit->len += next_lit->len;
          if (parent_node) {
            parent_node->next = prev_lit;
          } else {
            result.first.node = prev_lit;
          }
          prev_node = prev_lit;
        } else {
          prev_node->next = next_node;
          parent_node = prev_node;
          prev_node = next_node;
        }
      }
      result.next.node = parent_node;
      return result;
    }
  }

  J_A(NODISC) static inline regex_expr reduce_unary(const regex_op & op, regex_expr && lhs) noexcept {
    J_ASSERT(op.type == op_rep);
    // J_DEBUG("Reducing repetition");
    bool lhs_is_anchor = lhs.type == expr_single
      && (lhs.first.node->type == nt_anchor_subject_begin
          || lhs.first.node->type == nt_anchor_subject_end);
    if (!op.min && lhs_is_anchor) {
      return regex_expr{};
    }
    if (lhs.type == expr_epsilon
        || (op.min && lhs_is_anchor)
        || (op.min == 1 && op.max == 1)) {
      return lhs;
    }
    if (lhs.type == expr_single && lhs.first.node->type == nt_repetition) {
      node_repetition *lhs_node = static_cast<node_repetition *>(lhs.first.node);
      if (!lhs_node->min || (!lhs_node->max && op.min < 2) || (lhs_node->min == lhs_node->max && op.min == op.max)) {
        lhs_node->min *= op.min;
        lhs_node->max *= op.max;
        return lhs;
      }
    }
    return ::new node_repetition(lhs.to_node(), op.min, op.max);
  }

  struct J_AT(HIDDEN) ast_builder {
    p::precedence_parser<regex_expr, regex_op, get_op_info, reduce_binary, reduce_unary> parser;

    J_A(AI,ND) inline ast_builder() {
      push_epsilon();
    }

    void push_epsilon() {
      parser.push_expr(regex_expr{});
    }

    void cat(node_base * J_AA(NN) target) {
      parser.push_op({op_cat});
      parser.push_expr(target);
    }

    void push_named_char_class(char name) {
      cat(::new node_char_class(char_class{named_char_class_mask(name), {}}, false));
    }

    void begin_alternate() {
      parser.push_op({ op_alt });
      parser.push_expr(regex_expr());
    }

    void begin_group() {
      parser.push_op({ op_cat });
      parser.push_left_paren();
      parser.push_expr(regex_expr());
    }
    void end_group() {
      parser.push_right_paren();
    }

    void push_char(char ch) {
      u8_t c = ch;
      cat(alloc_literal(&c, 1));
    }

    void push_named_rep(char named_rep) {
      switch (named_rep) {
      case '?': return push_rep(0, 1);
      case '*': return push_rep(0, 0);
      case '+': return push_rep(1, 0);
      }
    }
    void push_rep(u32_t num) {
      if (num == 1) {
        return;
      }
      parser.push_op({op_rep, num, num});
    }
    void push_rep(u32_t min, u32_t max) {
      if (min == 1 && max == 1) {
        return;
      }
      J_REQUIRE(!max || min <= max, "Inverted repetition \\{{},{}}", min, max);
      parser.push_op({op_rep, min, max});
    }
    void push_special(node_type nt) {
      cat(::new node_base{nt});
    }

    void push_literal(const u8_t * J_AA(NN) begin, const u8_t* J_AA(NN) end) {
      cat(alloc_literal(begin, end - begin));
    }

    J_A(RNN,NODISC,AI) inline node_base *build() {
      return parser.finish().to_node();
    }
  };

  static node_base * read_terminated(reader_pos & pos, bool is_root = false) {
    ast_builder b;
    const u8_t *m1 = nullptr, *m2 = nullptr, *match = nullptr;
    /*!stags:re2c format = 'const u8_t *@@ = nullptr;'; */
    for (;;) {
      /*!use:re2c
        "[" { b.cat(read_char_class(pos)); continue; }
        "|" { b.begin_alternate(); continue; }
        "(" { b.begin_group(); continue; }
        ")" { b.end_group(); continue; }
        "$" { b.push_special(nt_anchor_subject_end); continue; }
        "^" { b.push_special(nt_anchor_subject_begin); continue; }
        "." { b.push_special(nt_dot); continue; }
        NAMED_QUANTIFIER { b.push_named_rep(pos.cursor[-1]); continue; }
        "{" @m1 [1-9][0-9]* "}" {
          b.push_rep(parse_decimal_integer<u32_t>((const char*)m1, pos.cursor - m1 - 1));
          continue;
        }
        "{" @m1 [0-9]+ "," @m2 [0-9]* "}" {
          b.push_rep(parse_decimal_integer<u32_t>((const char*)m1, m2 - m1 - 1),
                     m2 == pos.cursor - 1 ? 0 : parse_decimal_integer<u32_t>((const char*)m2, pos.cursor - m2 - 1));
          continue;
        }
        ESC NAMED_CHAR_CLASS { b.push_named_char_class(pos.cursor[-1]); continue; }
        ESC @m1 NORMAL_ESCAPE RE_NORMAL* / NOT_QUANTIFIER_START { b.push_literal(m1, pos.cursor); continue; }
        @m1 RE_NORMAL+ / NOT_QUANTIFIER_START { b.push_literal(m1, pos.cursor); continue; }
        @m1 RE_NORMAL { b.push_literal(m1, pos.cursor); continue; }
        * { goto fail; }
        [\x00] { goto out; }
      */
    }
  out:
    return b.build();
  fail:
    J_THROW("Unexpected character");
  }

  node_base * read(strings::const_string_view str) {
    j::mem::unique_ptr<char[]> buffer;
    const char * cstr = nullptr;
    if (!str.size()) {
      cstr = "";
    } else if (str.back() == '\0') {
      cstr = str.data();
    } else if (!is_aligned(str.end(), 16U) && *str.end() == '\0') {
      cstr = str.data();
    } else {
      buffer = j::mem::unique_ptr<char[]>(::new char[str.size() + 1U]);
      const u32_t sz = str.size();
      ::j::memcpy(buffer.get(), str.data(), sz);
      buffer[sz] = '\0';
      cstr = buffer.get();
    }
    reader_pos pos{(u8_t*)cstr, (u8_t*)cstr};
    return read_terminated(pos, true);
  }
}
