#include "utils.hpp"
#include "lisp/env/environment.hpp"
#include "logging/global.hpp"

using namespace j::lisp::tests;
using namespace j::lisp;

TEST_SUITE_BEGIN("Lisp - Closures");

TEST_CASE("Basic - No bindings No args") {
  auto e = make_env();
  auto closure = run_code(e, "(defun f () (fn () 1))", "f");
  REQUIRE_UNARY(closure.is_closure_ref());
  auto res = closure.as_closure_ref()();
  REQUIRE_UNARY(res.is_i64());
  REQUIRE_UNARY(res.eq(lisp_i64{1}));
  res = closure.as_closure_ref()();
  REQUIRE_UNARY(res.eq(lisp_i64{1}));
}

TEST_CASE("Basic - No bindings 1 Arg") {
  auto e = make_env();
  auto closure = run_code(e, "(defun f () (fn (a) (+ a 1)))", "f");
  REQUIRE_UNARY(closure.is_closure_ref());
  auto res = closure.as_closure_ref()(lisp_i64(9));
  REQUIRE_UNARY(res.is_i64());
  REQUIRE_UNARY(res.eq(lisp_i64{10}));
  res = closure.as_closure_ref()(lisp_i64(-7));
  REQUIRE_UNARY(res.eq(lisp_i64{-6}));
}

TEST_CASE("Basic - Single arg binding") {
  auto e = make_env();
  auto closure = run_code(e, "(defun f (a) (fn () a))", "f", lisp_i64(11));
  REQUIRE_UNARY(closure.is_closure_ref());
  auto res = closure.as_closure_ref()();
  REQUIRE_UNARY(res.is_i64());
  REQUIRE_UNARY(res.eq(lisp_i64{11}));
  res = closure.as_closure_ref()();
  REQUIRE_UNARY(res.eq(lisp_i64{11}));
}

TEST_CASE("Basic - Single arg binding diff instances") {
  auto e = make_env();
  auto closures = run_code(e, "(defun f (a) (fn () a)) (defun p () (vec (f 1) (f 2)))", "p");
  REQUIRE_UNARY(closures.is_vec_ref());
  REQUIRE_EQ(closures.as_vec_ref()->size(), 2U);
  auto closure0 = closures.as_vec_ref()->at(0);
  auto closure1 = closures.as_vec_ref()->at(1);
  REQUIRE_UNARY(closure0.is_closure_ref());
  REQUIRE_UNARY(closure1.is_closure_ref());
  REQUIRE_UNARY(closure0.as_closure_ref().function() == closure1.as_closure_ref().function());
  REQUIRE_NE(closure0.as_closure_ref().ptr(), closure1.as_closure_ref().ptr());
  auto res0 = closure0.as_closure_ref()();
  REQUIRE_UNARY(res0.is_i64());
  REQUIRE_UNARY(res0.eq(lisp_i64{1}));
  auto res1 = closure1.as_closure_ref()();
  REQUIRE_UNARY(res1.is_i64());
  REQUIRE_UNARY(res1.eq(lisp_i64{2}));
}

TEST_CASE("Shared - Setter and getter") {
  auto e = make_env();
  auto closures = run_code(e, "(import %air) (defun f (a) (vec (fn () a) (fn (b) (set a b))))", "f", lisp_i64(8));
  REQUIRE_UNARY(closures.is_vec_ref());
  REQUIRE_EQ(closures.as_vec_ref()->size(), 2U);
  auto getter = closures.as_vec_ref()->at(0);
  auto setter = closures.as_vec_ref()->at(1);
  REQUIRE_UNARY(getter.is_closure_ref());
  REQUIRE_UNARY(setter.is_closure_ref());
  auto res = getter.as_closure_ref()();
  REQUIRE_UNARY(res.is_i64());
  REQUIRE_UNARY(res.eq(lisp_i64{8}));
  res = getter.as_closure_ref()();
  REQUIRE_UNARY(res.eq(lisp_i64{8}));
  auto setter_res = setter.as_closure_ref()(lisp_i64{-19});
  REQUIRE_UNARY(setter_res.is_i64());
  REQUIRE_UNARY(setter_res.eq(lisp_i64{-19}));

  res = getter.as_closure_ref()();
  REQUIRE_UNARY(res.is_i64());
  REQUIRE_UNARY(res.eq(lisp_i64{-19}));
}

TEST_CASE("Shared - Split") {
  auto e = make_env();
  auto closures = run_code(
    e,
    "(import %air)\n"
    "(defun f (a)"
    "  (vec"
    "    (let ((b 0))"
    "      (fn (x)"
    "        (setp (a (+ a x))"
    "              (b (+ b x)))"
    "        (vec a b)))"
    "    (let ((b 0))"
    "      (fn (x)"
    "        (setp (a (+ a x))"
    "              (b (+ b x)))"
    "        (vec a b)))))",
    "f", lisp_i64(8));

  REQUIRE_UNARY(closures.is_vec_ref());
  REQUIRE_EQ(closures.as_vec_ref()->size(), 2U);
  auto closure0 = closures.as_vec_ref()->at(0);
  auto closure1 = closures.as_vec_ref()->at(1);
  REQUIRE_UNARY(closure0.is_closure_ref());
  REQUIRE_UNARY(closure1.is_closure_ref());
  auto res = closure0.as_closure_ref()(lisp_i64(0));
  CAPTURE(res.type());
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(8)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(0)));

  res = closure1.as_closure_ref()(lisp_i64(0));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(8)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(0)));

  res = closure0.as_closure_ref()(lisp_i64(2));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(10)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(2)));

  res = closure0.as_closure_ref()(lisp_i64(0));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(10)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(2)));

  res = closure1.as_closure_ref()(lisp_i64(0));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(10)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(0)));

  res = closure1.as_closure_ref()(lisp_i64(3));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(13)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(3)));

  res = closure1.as_closure_ref()(lisp_i64(0));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(13)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(3)));

  res = closure0.as_closure_ref()(lisp_i64(0));
  REQUIRE_UNARY(res.is_vec_ref());
  REQUIRE_EQ(res.as_vec_ref()->size(), 2U);
  REQUIRE_UNARY(res.as_vec_ref()->at(0).eq(lisp_i64(13)));
  REQUIRE_UNARY(res.as_vec_ref()->at(1).eq(lisp_i64(2)));
}

TEST_CASE("Shared - Loop") {
  auto e = make_env();
  auto closures = run_code(
    e,
    "(import %air)\n"
    "(defun f (a)"
    "  (let ((res (make-vec 20))"
    "        (idx 0))"
    "    (do-until"
    "      (setp (res[idx]"
    "             (let ((b idx)) (fn () (+ a b))))"
    "           (idx (+ idx 1)))"
    "      (eq idx 20))"
    "    res))",
    "f", lisp_i64(8));
  REQUIRE_UNARY(closures.is_vec_ref());
  if (closures.as_vec_ref()->size() != 20) {
    J_DEBUG("{:016X}", closures.raw);
    e->heap.dump();
  }
  REQUIRE_EQ(closures.as_vec_ref()->size(), 20U);
  u32_t i = 0;
  for (auto & c : closures.as_vec_ref()->value()) {
    REQUIRE_UNARY(c.is_closure_ref());
    auto res = c.as_closure_ref()();
    REQUIRE_UNARY(res.is_i64());
    REQUIRE_UNARY(res.eq(lisp_i64(8 + i)));
    ++i;
  }
}

TEST_SUITE_END();
