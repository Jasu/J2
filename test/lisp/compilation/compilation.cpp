#include "utils.hpp"
#include "lisp/air/air_package.hpp"

using namespace j::lisp::tests;
using namespace j::lisp;

TEST_SUITE_BEGIN("Lisp - Compilation");

TEST_CASE("Return nil") {
  REQUIRE_UNARY(run_code("(defun f () nil)", "f").eq(lisp_nil{}));
}
TEST_CASE("Return true") {
  REQUIRE_UNARY(run_code("(defun f () true)", "f").eq(lisp_true_v));
}
TEST_CASE("Return false") {
  REQUIRE_UNARY(run_code("(defun f () false)", "f").eq(lisp_false_v));
}

TEST_CASE("Return zero") {
  REQUIRE_UNARY(run_code("(defun f () 0)", "f").eq(lisp_i64{0}));
}
TEST_CASE("Return one") {
  REQUIRE_UNARY(run_code("(defun f () 1)", "f").eq(lisp_i64{1}));
}
TEST_CASE("Return minus one") {
  REQUIRE_UNARY(run_code("(defun f () -1)", "f").eq(lisp_i64{-1}));
}

TEST_CASE("Return empty string") {
  REQUIRE_EQ(run_code("(defun f () \"\")", "f").as_str_ref()->value(), "");
}
TEST_CASE("Return string") {
  REQUIRE_EQ(run_code("(defun f () \"Hello, World!\")", "f").as_str_ref()->value(), "Hello, World!");
}

TEST_CASE("Return empty vec") {
  auto res = run_code("(defun f () (vec))", "f");
  REQUIRE_EQ(res.as_vec_ref()->size(), 0);
  REQUIRE_EQ(res.raw, air::g_empty_vec_ref.raw);
}

TEST_CASE("Return (vec 999)") {
  auto res = run_code("(defun f () (vec 999))", "f");
  REQUIRE_EQ(res.as_vec_ref()->size(), 1);
  REQUIRE_UNARY(res.as_vec_ref()[0].eq(lisp_i64{999}));
}


TEST_CASE("Return only arg") {
  REQUIRE_UNARY(run_code("(defun f (a) a)", "f", lisp_i64{91}).eq(lisp_i64{91}));
}
TEST_CASE("Return first arg") {
  REQUIRE_UNARY(run_code("(defun f (a b) a)", "f", lisp_i64{92}, lisp_i64{93}).eq(lisp_i64{92}));
  REQUIRE_UNARY(run_code("(defun f (a b) a)", "f", lisp_i64{92}, lisp_i64{93}, lisp_i64{94}).eq(lisp_i64{92}));
}
TEST_CASE("Return second arg") {
  REQUIRE_UNARY(run_code("(defun f (a b) b)", "f", lisp_i64{92}, lisp_i64{93}).eq(lisp_i64{93}));
  REQUIRE_UNARY(run_code("(defun f (a b) b)", "f", lisp_i64{92}, lisp_i64{93}, lisp_i64{94}).eq(lisp_i64{93}));
}

TEST_SUITE_END();
