#include <detail/preamble.hpp>

#include "lisp/env/environment.hpp"
#include "lisp/env/compilation_options.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "lisp/reader/read.hpp"
#include "lisp/reader/tokens.hpp"

TEST_SUITE_BEGIN("Lisp - Reader");

namespace l = j::lisp;
namespace r = j::lisp::reader;
namespace s = j::strings;

struct J_TYPE_HIDDEN node;

struct J_TYPE_HIDDEN vec_data final {
  J_ALWAYS_INLINE vec_data() noexcept = default;

  void add_node(node & n) noexcept;

  template<typename... Ts>
  explicit vec_data(node && n, Ts && ... nodes);

  u32_t size;
  node * nodes;
};

struct J_TYPE_HIDDEN sym_data final {
  u8_t package_id = 0U;
  s::const_string_view symbol_name;
  s::const_string_view package_name;
};

struct J_TYPE_HIDDEN node final {
  l::imm_tag type = l::tag_i64;

  union {
    i64_t i;
    float f;
    sym_data sym;
    s::const_string_view s;
    vec_data l;
    bool b;
  } value = { 0 };

  J_ALWAYS_INLINE node() noexcept = default;

  J_ALWAYS_INLINE node(node && n) noexcept {
    J_MEMCPY_INLINE(this, &n, sizeof(node));
    n.type = l::tag_i64;
  }

  node & operator=(node && n) noexcept {
    J_MEMCPY_INLINE(this, &n, sizeof(node));
    n.type = l::tag_i64;
    return *this;
  }

  J_ALWAYS_INLINE node(l::imm_tag t) noexcept
    : type(t)
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, bool b) noexcept
    : type(t),
      value{ .b = b }
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, i64_t i) noexcept
    : type(t),
      value{ .i = i }
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, float f) noexcept
    : type(t),
      value{ .f = f }
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, s::const_string_view s) noexcept
    : type(t),
      value{ .s = s }
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, sym_data sym) noexcept
    : type(t),
      value{ .sym = sym }
  { }

  J_ALWAYS_INLINE node(l::imm_tag t, vec_data l) noexcept
    : type(t),
      value{ .l = static_cast<vec_data &&>(l) }
  { }

  J_ALWAYS_INLINE ~node() {
    if (type == l::tag_vec_ref) {
      ::delete[] value.l.nodes;
    }
  }
};

void vec_data::add_node(node & n) noexcept {
    nodes[size++] = static_cast<node &&>(n);
  }
template<typename... Ts>
vec_data::vec_data(node && n, Ts && ... nodes)
  : size(0),
    nodes(::new node[1U + sizeof...(Ts)])
{
  add_node(n);
  (add_node(nodes), ...);
}

template<typename... Ts>
inline node vec(Ts && ... nodes) {
  return { l::tag_vec_ref, vec_data(static_cast<Ts &&>(nodes)...) };
}
inline node integer(i64_t value) {
  return { l::tag_i64, value };
}
inline node boolean(bool value) {
  return { l::tag_bool, value };
}
inline node nil() {
  return { l::tag_nil };
}
inline node floating_point(float value) {
  return { l::tag_f32, value };
}
inline node local_symbol(s::const_string_view value) {
  return { l::tag_sym_id, sym_data{l::package_id_min_user, value, ""} };
}
inline node global_symbol(s::const_string_view value) {
  return { l::tag_sym_id, sym_data{l::package_id_global, value, ""} };
}
inline node qualified_symbol(s::const_string_view pkg, s::const_string_view value) {
  return { l::tag_sym_id, sym_data{l::package_id_min_user, value, pkg} };
}
inline node keyword(s::const_string_view value) {
  return { l::tag_sym_id, sym_data{l::package_id_keyword, value, ""} };
}
inline node string(s::const_string_view value) {
  return { l::tag_str_ref, value };
}

struct J_TYPE_HIDDEN checker final {
  const l::env::environment & e;
  const l::packages::pkg * p;

  void operator()(const l::lisp_sym_id & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(expected.value.sym.package_id == n.value().package_id());
    if (expected.value.sym.package_name) {
      REQUIRE_UNARY(n.value().is_unresolved() && !n.value().is_gensym());
      auto pair = p->symbol_table.names.get_unresolved_names(n.value());
      REQUIRE(*pair.sym_name == expected.value.sym.symbol_name);
      REQUIRE(*pair.pkg_name == expected.value.sym.package_name);
    } else {
      REQUIRE_UNARY(!n.value().is_unresolved() && !n.value().is_gensym());
      REQUIRE(*e.symbol_name(n.value()) == expected.value.sym.symbol_name);
    }
  }
  void operator()(const l::lisp_i64 & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(n.value() == expected.value.i);
  }
  void operator()(const l::lisp_f32 & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(n.value() == expected.value.f);
  }
  void operator()(const l::lisp_str_ref & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(*n.value() == expected.value.s);
  }
  void operator()(const l::lisp_vec_ref & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(n.value()->size() == expected.value.l.size);
    for(u32_t i = 0, sz = n.value()->size(); i < sz; ++i) {
      (*n.value())[i].visit(*this, expected.value.l.nodes[i]);
    }
  }
  void operator()(const l::lisp_rec_ref &, const node &) const {
    J_ASSERT_FAIL("Unexpected rec");
  }
  void operator()(const l::lisp_fn_ref &, const node &) const {
    J_ASSERT_FAIL("Unexpected func");
  }
  void operator()(const l::lisp_bool & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
    REQUIRE(n.value() == expected.value.b);
  }
  void operator()(const l::lisp_nil & n, const node & expected) const {
    REQUIRE(n.type() == expected.type);
  }
};

static void assert_ast_by_node(const l::env::environment & e, l::lisp_vec * ast, const node & expected) {
  checker c{e};
  auto ptr = l::lisp_vec_ref(ast);
  ptr.visit(c, expected);
}

template<typename... Nodes>
static void assert_ast(const l::packages::pkg_compiler & e, l::lisp_vec * ast, Nodes && ... nodes) {
  assert_ast_by_node(e, ast, vec(static_cast<Nodes &&>(nodes)...));
}

template<typename... Nodes>
static void assert_ast(s::const_string_view str, Nodes && ... nodes) {
  l::env::environment e;
  auto guard = e.enter();
  l::env::compilation_options opts;
  auto ec = e.get_env_compiler(&opts);
  auto ce = ec->create_pkg("pkg");
  ce->load_string_source("test", str);
  assert_ast_by_node(*ce, ce->ast, vec(static_cast<Nodes &&>(nodes)...));
}

TEST_CASE("Empty string") {
  assert_ast("");
}

TEST_CASE("Whitespace") {
  assert_ast(" ");
}

TEST_CASE("Comments") {
  assert_ast("; Hello");
}

TEST_CASE("WS + Comments") {
  assert_ast("  ; Hello");
}

inline void assert_int(s::const_string_view str, i64_t i) {
  assert_ast(str, integer(i));
}

TEST_CASE("Decimal integer tokens") {
  SUBCASE("Zero") { assert_int("0", 0); }
  SUBCASE("Minus zero") {
    assert_int("-0", 0);
  }
  SUBCASE("Plus zero") {
    assert_int("+0", 0);
  }
  SUBCASE("One") {
    assert_int("1", 1);
  }
  SUBCASE("Thousand") {
    assert_int("1000", 1000);
  }
  SUBCASE("Minus one") {
    assert_int("-1", -1);
  }
  SUBCASE("Minus thousand") {
    assert_int("-1000", -1000);
  }
  SUBCASE("Plus one") {
    assert_int("+1", 1);
  }
  SUBCASE("Plus thousand") {
    assert_int("+1000", 1000);
  }

  SUBCASE("Separated") {
    assert_int("1_456_123", 1456123);
  }

  SUBCASE("Plus separated") {
    assert_int("+1_456_123", 1456123);
  }

  SUBCASE("Minus separated") {
    assert_int("-1_456_123", -1456123);
  }
}

TEST_CASE("Hex integer tokens") {
  SUBCASE("Zero") {
    assert_int("0x0", 0);
  }
  SUBCASE("Minus zero") {
    assert_int("-0x0", 0);
  }
  SUBCASE("Plus zero") {
    assert_int("+0x0", 0);
  }
  SUBCASE("One") {
    assert_int("0x1", 1);
  }
  SUBCASE("Thousand") {
    assert_int("0x1000", 0x1000);
  }
  SUBCASE("Minus one") {
    assert_int("-0x1", -1);
  }
  SUBCASE("Minus thousand") {
    assert_int("-0x1000", -0x1000);
  }
  SUBCASE("Plus one") {
    assert_int("+0x1", 1);
  }
  SUBCASE("Plus thousand") {
    assert_int("+0x1000", 0x1000);
  }

  SUBCASE("Separated") {
    assert_int("0x1_456_123", 0x1456123);
  }

  SUBCASE("Plus separated") {
    assert_int("+0x1_456_123", 0x1456123);
  }

  SUBCASE("Minus separated") {
    assert_int("-0x1_456_123", -0x1456123);
  }
  SUBCASE("Lower case") {
    assert_int("0xabcdef12", 0xABCDEF12);
  }

  SUBCASE("Upper case") {
    assert_int("0xABCDEF12", 0xABCDEF12);
  }
}

TEST_CASE("Bin integer tokens") {
  SUBCASE("Zero") {
    assert_int("0b0", 0);
  }
  SUBCASE("Minus zero") {
    assert_int("-0b0", 0);
  }
  SUBCASE("Plus zero") {
    assert_int("+0b0", 0);
  }
  SUBCASE("One") {
    assert_int("0b1", 1);
  }
  SUBCASE("Thousand") {
    assert_int("0b1000", 0b1000);
  }
  SUBCASE("Minus one") {
    assert_int("-0b1", -1);
  }
  SUBCASE("Minus thousand") {
    assert_int("-0b1000", -0b1000);
  }
  SUBCASE("Plus one") {
    assert_int("+0b1", 1);
  }
  SUBCASE("Plus thousand") {
    assert_int("+0b1000", 0b1000);
  }

  SUBCASE("Separated") {
    assert_int("0b10_110_011", 0b10110011);
  }

  SUBCASE("Plus separated") {
    assert_int("+0b010_110_011", 0b10110011);
  }

  SUBCASE("Minus separated") {
    assert_int("-0b010_110_011", -0b10110011);
  }
}

TEST_CASE("Fixed-value tokens") {
  SUBCASE("true") { assert_ast("true", boolean(true)); }
  SUBCASE("false") { assert_ast("false", boolean(false)); }
  SUBCASE("nil") { assert_ast("nil", nil()); }
}


inline static void test_symbol(s::const_string_view id) {
  assert_ast(id, local_symbol(id));
}
inline static void test_global_symbol(s::const_string_view id) {
  assert_ast(id, global_symbol(id));
}

TEST_CASE("Symbol tokens") {
  SUBCASE("hello-world") { test_symbol("hello-world"); }
  SUBCASE("a")           { test_symbol("a"); }
  SUBCASE("A")           { test_symbol("A"); }
  SUBCASE("a0")          { test_symbol("a0"); }
  SUBCASE("x1")          { test_symbol("x1"); }
  SUBCASE("b2")          { test_symbol("b2"); }
  SUBCASE("-")           { test_global_symbol("-"); }
  SUBCASE("+")           { test_global_symbol("+"); }
  SUBCASE("++")          { test_symbol("++"); }
  SUBCASE("--")          { test_symbol("--"); }
  SUBCASE("-a0")         { test_symbol("-a0"); }
  SUBCASE("+a1")         { test_symbol("+a1"); }
  SUBCASE("--2")         { test_symbol("--2"); }
  SUBCASE("++3")         { test_symbol("++3"); }
}

inline static void test_qualified_symbol(s::const_string_view str, s::const_string_view pkg, s::const_string_view sym) {
  assert_ast(str, qualified_symbol(pkg, sym));
}

TEST_CASE("qualified symbol tokens") {
  SUBCASE("a:a")                     { test_qualified_symbol("a:a", "a", "a"); }
  SUBCASE("a:b")                     { test_qualified_symbol("a:b", "a", "b"); }
  SUBCASE("hello-world:and-goodbye") { test_qualified_symbol("hello-world:and-goodbye", "hello-world", "and-goodbye"); }
  SUBCASE("-a:-b")                   { test_qualified_symbol("-a:-b", "-a", "-b"); }
}

inline static void test_keyword(s::const_string_view kwd) {
  assert_ast(kwd, keyword(kwd.without_prefix(1)));
}

TEST_CASE("Keyword tokens") {
  SUBCASE(":hello-world") { test_keyword(":hello-world"); }
  SUBCASE(":a")           { test_keyword(":a"); }
  SUBCASE(":A")           { test_keyword(":A"); }
  SUBCASE(":a0")          { test_keyword(":a0"); }
  SUBCASE(":x1")          { test_keyword(":x1"); }
  SUBCASE(":b2")          { test_keyword(":b2"); }
  SUBCASE(":-")           { test_keyword(":-"); }
  SUBCASE(":+")           { test_keyword(":+"); }
  SUBCASE(":++")          { test_keyword(":++"); }
  SUBCASE(":--")          { test_keyword(":--"); }
  SUBCASE(":-a0")         { test_keyword(":-a0"); }
  SUBCASE(":+a1")         { test_keyword(":+a1"); }
  SUBCASE(":--2")         { test_keyword(":--2"); }
  SUBCASE(":++3")         { test_keyword(":++3"); }

  SUBCASE(":3")           { test_keyword(":3"); }
  SUBCASE(":-3")          { test_keyword(":-3"); }
}

void string_case(s::const_string_view in, s::const_string_view out = {}) {
  if (!out) {
    out = in;
    out.remove_prefix(1);
    out.remove_suffix(1);
  }
  assert_ast(in, string(out));
}

#define Q "\""
#define E "\\"
TEST_CASE("String tokens") {
  SUBCASE("Empty")                { string_case(Q Q); }
  SUBCASE("Single char")          { string_case(Q "A" Q); }
  SUBCASE("Multiple chars")       { string_case(Q "Hello, world!" Q); }

  SUBCASE("Escaped quote")        { string_case(Q E Q Q, Q); }
  SUBCASE("Escaped backslash")    { string_case(Q E E Q, E); }
  SUBCASE("Escaped LF")           { string_case(Q E "n" Q, "\n"); }
  SUBCASE("Escaped CR")           { string_case(Q E "r" Q, "\r"); }
  SUBCASE("Escaped ESC")          { string_case(Q E "e" Q, "\x1B"); }
  SUBCASE("Escaped bell")         { string_case(Q E "a" Q, "\a"); }
  SUBCASE("Escaped backspace")    { string_case(Q E "b" Q, "\b"); }
  SUBCASE("Escaped form feed")    { string_case(Q E "f" Q, "\f"); }
  SUBCASE("Escaped tab")          { string_case(Q E "t" Q, "\t"); }
  SUBCASE("Escaped vertical tab") { string_case(Q E "v" Q, "\v"); }
  SUBCASE("Escaped NUL")          { string_case(Q E "0" Q, s::const_string_view("\0", 1)); }

  SUBCASE("Hex NUL")              { string_case(Q E "x00" Q, s::const_string_view("\0", 1)); }
  SUBCASE("Hex one")              { string_case(Q E "x01" Q, "\x01"); }
  SUBCASE("Hex 0x80")             { string_case(Q E "x80" Q, "\x80"); }
  SUBCASE("Hex 0xF0")             { string_case(Q E "xF0" Q, "\xF0"); }
  SUBCASE("Hex 0xFF")             { string_case(Q E "xFF" Q, "\xFF"); }
  SUBCASE("Hex lower 1")          { string_case(Q E "xab" Q, "\xAB"); }
  SUBCASE("Hex lower 2")          { string_case(Q E "xcd" Q, "\xCD"); }
  SUBCASE("Hex lower 3")          { string_case(Q E "xef" Q, "\xEF"); }

  SUBCASE("Short UTF-8 U+0000")   { string_case(Q E "u0000" Q, s::const_string_view("\0", 1)); }
  SUBCASE("Short UTF-8 U+007F")   { string_case(Q E "u007F" Q, "\x7F"); }
  SUBCASE("Short UTF-8 U+0080")   { string_case(Q E "u0080" Q, "\u0080"); }
  SUBCASE("Short UTF-8 U+07FF")   { string_case(Q E "u07FF" Q, "\u07FF"); }
  SUBCASE("Short UTF-8 U+0800")   { string_case(Q E "u0800" Q, "\u0800"); }
  SUBCASE("Short UTF-8 U+FFFF")   { string_case(Q E "uFFFF" Q, "\uFFFF"); }

  SUBCASE("Short UTF-8 lower")    { string_case(Q E "uabcd" Q, "\uabcd"); }
  SUBCASE("Short UTF-8 lower 2")  { string_case(Q E "uef01" Q, "\uef01"); }

  SUBCASE("Long UTF-8 U+0000")   { string_case(Q E "U00000000" Q, s::const_string_view("\0", 1)); }
  SUBCASE("Long UTF-8 U+007F")   { string_case(Q E "U0000007F" Q, "\x7F"); }
  SUBCASE("Long UTF-8 U+0080")   { string_case(Q E "U00000080" Q, "\u0080"); }
  SUBCASE("Long UTF-8 U+07FF")   { string_case(Q E "U000007FF" Q, "\u07FF"); }
  SUBCASE("Long UTF-8 U+0800")   { string_case(Q E "U00000800" Q, "\u0800"); }
  SUBCASE("Long UTF-8 U+FFFF")   { string_case(Q E "U0000FFFF" Q, "\uFFFF"); }

  SUBCASE("Long UTF-8 U+010000") { string_case(Q E "U00010000" Q, "\U00010000"); }
  SUBCASE("Long UTF-8 U+102181") { string_case(Q E "U001021Ff" Q, "\U001021Ff"); }
  SUBCASE("Long UTF-8 lower")    { string_case(Q E "U000abcde" Q, "\U000abcde"); }

  SUBCASE("Escaped escape")      { string_case(Q E E "n" Q, "\\n"); }
  SUBCASE("Escaped in middle")   { string_case(Q "Hello,\\nWorld" Q, "Hello,\nWorld"); }
}
#undef Q
#undef E

inline static void float_case(s::const_string_view in, double d) {
  assert_ast(in, floating_point(d));
}

TEST_CASE("Test floating point") {
  SUBCASE("0.0")        { float_case("0.0",        0.0); }
  SUBCASE(".0")         { float_case(".0",         0.0); }
  SUBCASE("0_0.0_0")    { float_case("0_0.0_0",    0.0); }
  SUBCASE("0e1")        { float_case("0e1",        0.0); }
  SUBCASE("0e-1")       { float_case("0e-1",       0.0); }
  SUBCASE("0E1")        { float_case("0E1",        0.0); }
  SUBCASE("0E-1")       { float_case("0E-1",       0.0); }

  SUBCASE("0.5")        { float_case("0.5",        0.5); }
  SUBCASE(".5")         { float_case(".5",         0.5); }
  SUBCASE("0_0.5_0")    { float_case("0_0.5_0",    0.5); }
  SUBCASE("5.0e-1")     { float_case("5.0e-1",     0.5); }
  SUBCASE("5.0E-1")     { float_case("5.0E-1",     0.5); }
  SUBCASE("5e-1")       { float_case("5e-1",       0.5); }
  SUBCASE("5E-1")       { float_case("5E-1",       0.5); }

  SUBCASE("-0.5")       { float_case("-0.5",       -0.5); }
  SUBCASE("-.5")        { float_case("-.5",        -0.5); }
  SUBCASE("-0_0.5_0")   { float_case("-0_0.5_0",   -0.5); }
  SUBCASE("-5.0e-1")    { float_case("-5.0e-1",    -0.5); }
  SUBCASE("-5.0E-1")    { float_case("-5.0E-1",    -0.5); }
  SUBCASE("-5e-1")      { float_case("-5e-1",      -0.5); }
  SUBCASE("-5E-1")      { float_case("-5E-1",      -0.5); }

  SUBCASE("201.25")     { float_case("201.25",     201.25); }
  SUBCASE("20125E-2")   { float_case("20125E-2",   201.25); }
  SUBCASE("20_125E-2")  { float_case("20_125E-2",  201.25); }

  SUBCASE("-201.25")    { float_case("-201.25",    -201.25); }
  SUBCASE("-20125E-2")  { float_case("-20125E-2",  -201.25); }
  SUBCASE("-20_125E-2") { float_case("-20_125E-2", -201.25); }
}

TEST_CASE("Paren stack") {
  assert_ast(
    "(1 (- 2 3 (4 5) 6) (8) ()) (() (2))",
    vec(
      integer(1),
      vec(
        global_symbol("-"),
        integer(2),
        integer(3),
        vec(integer(4), integer(5)),
        integer(6)),
      vec(integer(8)),
      vec()),

    vec(
      vec(),
      vec(integer(2)))
    );
}

TEST_CASE("Complex") {
  assert_ast(
    ";; This is a test.\n"
    "  (- 1 (* +7 *HELLO*)); Comment\r"
    "(print \"Test\\n\" 12.125 -99 :frooba)\n"
    "(eq 'a `(hello ,bar))\n"
    "(frooba:bar true false nil) "
    "progn "
    "...pre post... ...both... "
    "...:pre :post... ...:both... "
    "...frooba:pre frooba:post... ...frooba:both... ",
    vec(
      global_symbol("-"),
      integer(1),
      vec(
        global_symbol("*"),
        integer(7),
        local_symbol("*HELLO*"))),
    vec(
      local_symbol("print"),
      string("Test\n"),
      floating_point(12.125),
      integer(-99),
      keyword("frooba")),
    vec(
      global_symbol("eq"),
      vec(
        global_symbol("quote"),
        local_symbol("a")),
      vec(
        global_symbol("quasiquote"),
        vec(
          local_symbol("hello"),
          vec(global_symbol("unquote"), local_symbol("bar"))))),
    vec(
      qualified_symbol("frooba", "bar"),
      boolean(true),
      boolean(false),
      nil()),
    global_symbol("progn"),

    vec(global_symbol("pre-ellipsis"), local_symbol("pre")),
    vec(global_symbol("post-ellipsis"), local_symbol("post")),
    vec(global_symbol("pre-ellipsis"),
        vec(global_symbol("post-ellipsis"), local_symbol("both"))),

    vec(global_symbol("pre-ellipsis"), keyword("pre")),
    vec(global_symbol("post-ellipsis"), keyword("post")),
    vec(global_symbol("pre-ellipsis"),
        vec(global_symbol("post-ellipsis"), keyword("both"))),

    vec(global_symbol("pre-ellipsis"), qualified_symbol("frooba", "pre")),
    vec(global_symbol("post-ellipsis"), qualified_symbol("frooba", "post")),
    vec(global_symbol("pre-ellipsis"),
        vec(global_symbol("post-ellipsis"), qualified_symbol("frooba", "both")))
    );
}
TEST_SUITE_END();
