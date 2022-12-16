#include "utils.hpp"

using namespace j::lisp::tests;
using namespace j::lisp;

TEST_SUITE_BEGIN("Lisp - Arithmetic");

#define INT_UNOP_CASE(OPERATOR, V, EXPECT) SUBCASE("(" #OPERATOR " " #V ")") { \
  REQUIRE_UNARY(run_code("(defun f () (" #OPERATOR " " #V "))", "f").eq(lisp_i64{EXPECT})); \
  REQUIRE_UNARY(run_code("(defun f (v) (" #OPERATOR " v))", "f", lisp_i64{V}).eq(lisp_i64{EXPECT})); \
}

#define INT_BINOP_CASE(OPERATOR, LHS, RHS, EXPECT) SUBCASE("(" #OPERATOR " " #LHS " " #RHS ")") { \
  REQUIRE_UNARY(run_code("(defun f () (" #OPERATOR " " #LHS " " #RHS "))", "f").eq(lisp_i64{EXPECT})); \
  REQUIRE_UNARY(run_code("(defun f (lhs) (" #OPERATOR " lhs " #RHS "))", "f", lisp_i64{LHS}).eq(lisp_i64{EXPECT})); \
  REQUIRE_UNARY(run_code("(defun f (rhs) (" #OPERATOR " " #LHS " rhs))", "f", lisp_i64{RHS}).eq(lisp_i64{EXPECT})); \
  REQUIRE_UNARY(run_code("(defun f (lhs rhs) (" #OPERATOR " lhs rhs))", "f", lisp_i64{LHS}, lisp_i64{RHS}).eq(lisp_i64{EXPECT})); \
}

#define INT_EQ_CASE(EXPR, EXPECT) SUBCASE(EXPR) { \
  REQUIRE_UNARY(run_code("(defun f () " EXPR ")", "f").eq(lisp_i64{EXPECT})); \
}

TEST_CASE("iadd") {
  INT_BINOP_CASE(+, 6,  2,  8)
  INT_BINOP_CASE(+, 0,  1,  1)
  INT_BINOP_CASE(+, 0,  -1, -1)
  INT_BINOP_CASE(+, 1,  1,  2)
  INT_BINOP_CASE(+, 1,  -1, 0)
  INT_BINOP_CASE(+, -1, 1,  0)
  INT_BINOP_CASE(+, -1, -1, -2)

  INT_UNOP_CASE(+, 1, 1)
  INT_UNOP_CASE(+, -1, -1)
  INT_UNOP_CASE(+, 10, 10)
  INT_UNOP_CASE(+, 0, 0)

  INT_EQ_CASE("(+ 40 2 5)", 47)
}

TEST_CASE("ineg") {
  INT_UNOP_CASE(-, 1, -1)
  INT_UNOP_CASE(-, -1, 1)
  INT_UNOP_CASE(-, 10, -10)
  INT_UNOP_CASE(-, 0, 0)
}

TEST_CASE("isub") {
  INT_BINOP_CASE(-, 6,  2,  4)
  INT_BINOP_CASE(-, 0,  1,  -1)
  INT_BINOP_CASE(-, 0,  -1, 1)
  INT_BINOP_CASE(-, 1,  1,  0)
  INT_BINOP_CASE(-, 1,  -1, 2)
  INT_BINOP_CASE(-, -1, 1,  -2)
  INT_BINOP_CASE(-, -1, -1, 0)

  INT_EQ_CASE("(- 40 2 5)", 33)
}

TEST_CASE("imul") {
  INT_BINOP_CASE(*, 6,  2,  12)
  INT_BINOP_CASE(*, 0,  1,  0)
  INT_BINOP_CASE(*, 0,  -1, 0)
  INT_BINOP_CASE(*, 1,  1,  1)
  INT_BINOP_CASE(*, 1,  -1, -1)
  INT_BINOP_CASE(*, -1, 1,  -1)
  INT_BINOP_CASE(*, -1, -1, 1)

  INT_UNOP_CASE(*, 1, 1)
  INT_UNOP_CASE(*, -1, -1)
  INT_UNOP_CASE(*, 10, 10)
  INT_UNOP_CASE(*, 0, 0)

  INT_EQ_CASE("(* 40 2 5)", 400)
}

TEST_CASE("idiv") {
  INT_BINOP_CASE(/, 6,  2,  3)
  INT_BINOP_CASE(/, 0,  1,  0)
  INT_BINOP_CASE(/, 0,  -1, 0)
  INT_BINOP_CASE(/, 1,  1,  1)
  INT_BINOP_CASE(/, 1,  -1, -1)
  INT_BINOP_CASE(/, -1, 1,  -1)
  INT_BINOP_CASE(/, -1, -1, 1)

  INT_UNOP_CASE(/, 1, 1)
  INT_UNOP_CASE(/, -1, -1)
  INT_UNOP_CASE(/, 10, 10)
  INT_UNOP_CASE(/, 0, 0)

  INT_EQ_CASE("(/ 40 2 5)", 4)
}

TEST_SUITE_END();
