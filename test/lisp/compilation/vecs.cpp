#include "utils.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/mem/heap.hpp"

using namespace j::lisp::tests;
using namespace j::lisp;

TEST_SUITE_BEGIN("Lisp - Vecs");

inline static void require_vec_elem_eq(u32_t idx, imms_t vec, lisp_imm expect) {
  u32_t sz = vec.size();
  CAPTURE(idx);
  CAPTURE(sz);
  REQUIRE_LT(idx, sz);
  CAPTURE(vec[idx].raw);
  CAPTURE(expect.raw);
  REQUIRE_UNARY(vec[idx].eq(expect));
}

template<typename... Elems>
static void require_vec_eq(lisp_imm imm, Elems... elems) {
  REQUIRE_UNARY(imm.is_vec_ref());
  auto vec = imm.as_vec_ref()->value();
  u32_t i = 0U;
  (require_vec_elem_eq(i++, vec, elems), ...);
  REQUIRE_EQ(vec.size(), sizeof...(elems));
}

#define EMPTY_VEC_CASE(CODE, ...) EQ_CASE(CODE, air::g_empty_vec_ref)

#define SHALLOW_VEC_CASE(CODE, ...) SUBCASE(CODE) { \
  require_vec_eq(run_code(e, "(defun f () " CODE ")", "f") __VA_OPT__(,) __VA_ARGS__); \
}

#define SHALLOW_VEC_CASE_1(CODE, A, ...) SUBCASE(CODE) {            \
  require_vec_eq(run_code(e, "(defun f (a) " CODE ")", "f", A) __VA_OPT__(,) __VA_ARGS__); \
}

#define SHALLOW_VEC_CASE_2(CODE, A, B, ...) SUBCASE(CODE) {               \
  require_vec_eq(run_code(e, "(defun f (a b) " CODE ")", "f", A, B) __VA_OPT__(,) __VA_ARGS__); \
}

TEST_CASE("vec") {
  auto e = make_env();
  EMPTY_VEC_CASE("(vec)", lisp_nil{})

  SHALLOW_VEC_CASE("(vec nil)", lisp_nil{})
  SHALLOW_VEC_CASE("(vec nil nil)", lisp_nil{}, lisp_nil{})
  SHALLOW_VEC_CASE("(vec 1 2)", lisp_i64{1}, lisp_i64{2})
  SHALLOW_VEC_CASE("(vec true false)", lisp_true_v, lisp_false_v)
  SHALLOW_VEC_CASE("(vec (vec) (vec))", air::g_empty_vec_ref, air::g_empty_vec_ref)

  SHALLOW_VEC_CASE_1("(vec a (vec))", lisp_i64{2}, lisp_i64{2}, air::g_empty_vec_ref)
  SHALLOW_VEC_CASE_1("(vec (vec) a)", lisp_i64{2}, air::g_empty_vec_ref, lisp_i64{2})
  SHALLOW_VEC_CASE_1("(vec a a)", lisp_i64{2}, lisp_i64{2}, lisp_i64{2})

  SHALLOW_VEC_CASE_2("(vec a b)", lisp_i64{-1}, lisp_true_v, lisp_i64{-1}, lisp_true_v )
  SHALLOW_VEC_CASE_2("(vec b a)", lisp_i64{3}, lisp_nil_v, lisp_nil_v, lisp_i64{3})
}

TEST_CASE("make-vec") {
  auto e = make_env();
  EMPTY_VEC_CASE("(make-vec 0)")
  SHALLOW_VEC_CASE("(make-vec 1)", lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 2)", lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 3)", lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 15)", lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                    lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 16)", lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                    lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 17)", lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                    lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE("(make-vec 18)", lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                    lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)

  SHALLOW_VEC_CASE_1("(make-vec a)", lisp_i64{1}, lisp_nil_v)
  SHALLOW_VEC_CASE_1("(make-vec a)", lisp_i64{15},
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE_1("(make-vec a)", lisp_i64{16},
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
  SHALLOW_VEC_CASE_1("(make-vec a)", lisp_i64{17},
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v,
                                     lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v, lisp_nil_v)
}

TEST_CASE("set") {
  auto e = make_env();
  SHALLOW_VEC_CASE("(let ((v (make-vec 1))) (set (at v 0) 8) v)", lisp_i64(8))
  SHALLOW_VEC_CASE("(let ((v (make-vec 2))) (set (at v 1) true) (set (at v 0) 8) v)", lisp_i64(8), lisp_true_v)
  SHALLOW_VEC_CASE_1("(let ((v (make-vec 1))) (set (at v 0) a) v)", lisp_i64(9), lisp_i64(9))
  SHALLOW_VEC_CASE_2("(let ((v (make-vec 2))) (set (at v 1) b) (set (at v 0) a) v)", lisp_true_v, lisp_false_v, lisp_true_v, lisp_false_v)

  SHALLOW_VEC_CASE("(let ((v (vec 1))) (set (at v 0) 8) v)", lisp_i64(8))
  SHALLOW_VEC_CASE("(let ((v (vec 3 2))) (set (at v 1) true) (set (at v 0) 8) v)", lisp_i64(8), lisp_true_v)
  SHALLOW_VEC_CASE_1("(let ((v (vec nil))) (set (at v 0) a) v)", lisp_i64(9), lisp_i64(9))
  SHALLOW_VEC_CASE_2("(let ((v (vec b a))) (set (at v 1) b) (set (at v 0) a) v)", lisp_true_v, lisp_false_v, lisp_true_v, lisp_false_v)

  SHALLOW_VEC_CASE("(let ((v (make-vec 1))) (set v[0] 8) v)", lisp_i64(8))
  SHALLOW_VEC_CASE("(let ((v (make-vec 2))) (setp ((at v 1) true) (v[0] 8)) v)", lisp_i64(8), lisp_true_v)
  SHALLOW_VEC_CASE_1("(let ((v (make-vec 1))) (set v[0] a) v)", lisp_i64(9), lisp_i64(9))
  SHALLOW_VEC_CASE_2("(let ((v (make-vec 2))) (set v[a] b) v)", lisp_i64(0), lisp_true_v, lisp_true_v, lisp_nil_v)
  SHALLOW_VEC_CASE_2("(let ((v (make-vec 2))) (setp (v[a] b)) v)", lisp_i64(1), lisp_false_v, lisp_nil_v, lisp_false_v)
  SHALLOW_VEC_CASE_2("(let ((v (make-vec a))) (set v[(- a 1)] b) v)", lisp_i64(1), lisp_false_v, lisp_false_v)
  SHALLOW_VEC_CASE_2("(let ((v (make-vec a))) (set v[(- a 1)] b) v)", lisp_i64(2), lisp_false_v, lisp_nil_v, lisp_false_v)
}

TEST_CASE("get") {
  auto e = make_env();
  EQ_CASE("(at (vec 1) 0)", lisp_i64{1})
  EQ_CASE("(let ((v (vec 7))) (at v 0))", lisp_i64{7})
  EQ_CASE("(let ((v (vec 7 8 9))) (at v 0))", lisp_i64{7})
  EQ_CASE("(let ((v (vec 7 8 9))) (at v 1))", lisp_i64{8})
  EQ_CASE("(let ((v (vec 7 8 9))) (at v 2))", lisp_i64{9})

  EQ_CASE("(let ((v (vec 7 8 9))) v[0])", lisp_i64{7})
  EQ_CASE("(let ((v (vec 7 8 9))) v[1])", lisp_i64{8})
  EQ_CASE("(let ((v (vec 7 8 9))) v[2])", lisp_i64{9})

  EQ_CASE_1("(let ((v (vec 7 8 9))) v[a])", lisp_i64{0}, lisp_i64{7})
  EQ_CASE_1("(let ((v (vec 7 8 9))) v[a])", lisp_i64{1}, lisp_i64{8})
  EQ_CASE_1("(let ((v (vec 7 8 9))) v[a])", lisp_i64{2}, lisp_i64{9})

  EQ_CASE("(at (make-vec 1) 0)", lisp_nil_v)
  EQ_CASE("(let ((v (make-vec 1))) (at v 0))", lisp_nil_v)
}

TEST_CASE("length") {
  auto e = make_env();
  EQ_CASE("(length (vec))", lisp_i64{0})
  EQ_CASE("(length (vec 1))", lisp_i64{1})
  EQ_CASE("(length (vec 1 2))", lisp_i64{2})
  EQ_CASE("(let ((v (vec))) (length v))", lisp_i64{0})
  EQ_CASE("(let ((v (vec 1))) (length v))", lisp_i64{1})
  EQ_CASE("(let ((v (vec 1 2))) (length v))", lisp_i64{2})

  EQ_CASE("(length (make-vec 0))", lisp_i64{0})
  EQ_CASE("(length (make-vec 1))", lisp_i64{1})
  EQ_CASE("(length (make-vec 2))", lisp_i64{2})
  EQ_CASE("(let ((v (make-vec 0))) (length v))", lisp_i64{0})
  EQ_CASE("(let ((v (make-vec 1))) (length v))", lisp_i64{1})
  EQ_CASE("(let ((v (make-vec 2))) (length v))", lisp_i64{2})

  EQ_CASE_1("(length (make-vec a))", lisp_i64{0}, lisp_i64{0})
  EQ_CASE_1("(length (make-vec a))", lisp_i64{1}, lisp_i64{1})
  EQ_CASE_1("(length (make-vec a))", lisp_i64{2}, lisp_i64{2})
  EQ_CASE_1("(let ((v (make-vec a))) (length v))", lisp_i64{0}, lisp_i64{0})
  EQ_CASE_1("(let ((v (make-vec a))) (length v))", lisp_i64{1}, lisp_i64{1})
  EQ_CASE_1("(let ((v (make-vec a))) (length v))", lisp_i64{2}, lisp_i64{2})
}

TEST_CASE("quasi") {
  auto e = make_env();

  EMPTY_VEC_CASE("`()")
  SHALLOW_VEC_CASE_1("`(0 ,a)", lisp_i64{2}, lisp_i64{0}, lisp_i64{2})
  SHALLOW_VEC_CASE_1("`(,a ,a ,a)", lisp_true_v, lisp_true_v, lisp_true_v, lisp_true_v)
  SHALLOW_VEC_CASE("`(0 ,...'())", lisp_i64{0})
  SHALLOW_VEC_CASE_1("`(0 ,...(vec a))", lisp_i64{1}, lisp_i64{0}, lisp_i64{1})
  SHALLOW_VEC_CASE_1("`(0 ,...'(1 2 3) ,a)", lisp_i64{4}, lisp_i64{0}, lisp_i64{1}, lisp_i64{2}, lisp_i64{3}, lisp_i64{4})
  lisp_imm data[]{lisp_i64{7}, lisp_nil_v, lisp_true_v};
  SHALLOW_VEC_CASE_1("`(,...a)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_1("`(,...a ,...a)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{7}, lisp_nil_v, lisp_true_v, lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_1("`(,...a ,...a)", air::g_empty_vec_ref)
  SHALLOW_VEC_CASE_1("(at `(1 2 (3 ,a) 4) 2)", lisp_true_v, lisp_i64(3), lisp_true_v)
  SHALLOW_VEC_CASE_1("(at `(1 2 (3 ,...a nil) 4) 2)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64(3), lisp_i64{7}, lisp_nil_v, lisp_true_v, lisp_nil_v)

  EQ_CASE_1("(length `(,...a))", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{3})
  EQ_CASE_1("(length `(,...a ,...a))", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{6})
  EQ_CASE_1("(length `(,...a ,...a))", air::g_empty_vec_ref, lisp_i64(0))
  EQ_CASE_1("(length (at `(1 2 (3 ,a) 4) 2))", lisp_true_v, lisp_i64(2))
  EQ_CASE_1("(length (at `(1 2 (3 ,...a nil) 4) 2))", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64(5))
}

TEST_CASE("copy") {
  auto e = make_env();
  SHALLOW_VEC_CASE("(let ((v (vec 1 2 3)) (v2 (copy v))) (set v[1] nil) (vec v[1] v2[1]))", lisp_nil_v, lisp_i64{2})
  SHALLOW_VEC_CASE("(let ((v (vec 1 2 3)) (v2 (copy v))) (set v2[1] 9) (vec v[1] v2[1]))", lisp_i64(2), lisp_i64{9})
  EQ_CASE("(length (copy (make-vec 0)))", lisp_i64(0))
  EQ_CASE("(length (copy (make-vec 2)))", lisp_i64(2))
  EQ_CASE_1("(length (copy (make-vec a)))", lisp_i64(0), lisp_i64(0))
  EQ_CASE_1("(length (copy (make-vec a)))", lisp_i64(2), lisp_i64(2))
}

TEST_CASE("append") {
  auto e = make_env();
  lisp_imm data[]{lisp_i64{7}, lisp_nil_v, lisp_true_v};
  SHALLOW_VEC_CASE("(append (vec 1 2 3) (vec 4 5))", lisp_i64(1), lisp_i64(2), lisp_i64(3), lisp_i64(4), lisp_i64(5))
  SHALLOW_VEC_CASE_1("(append a (vec 4 5))", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{7}, lisp_nil_v, lisp_true_v, lisp_i64(4), lisp_i64(5))
  SHALLOW_VEC_CASE_1("(append (vec 1 2) a)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64(1), lisp_i64(2), lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_1("(append (vec 1 2) a (vec 4 5))", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64(1), lisp_i64(2), lisp_i64{7}, lisp_nil_v, lisp_true_v, lisp_i64(4), lisp_i64(5))
  SHALLOW_VEC_CASE_1("(append a a)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), lisp_i64{7}, lisp_nil_v, lisp_true_v, lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_2("(append a b)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), air::g_empty_vec_ref, lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_2("(append b a)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), air::g_empty_vec_ref, lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_2("(append b a b)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), air::g_empty_vec_ref, lisp_i64{7}, lisp_nil_v, lisp_true_v)
  SHALLOW_VEC_CASE_2("(append b b b a b b b)", lisp_vec_ref(lisp_vec::allocate(e->heap, imms_t{data})), air::g_empty_vec_ref, lisp_i64{7}, lisp_nil_v, lisp_true_v)
}

TEST_SUITE_END();
