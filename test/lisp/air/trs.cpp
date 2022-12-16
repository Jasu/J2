// GENERATED FILE - EDITS WILL BE LOST.
#include <detail/preamble.hpp>

#include "lisp/air/exprs/exprs.hpp"
#include "lisp/air/exprs/write_expr.hpp"
#include "lisp/air/passes/trs_write_expr.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/packages/pkg.hpp"
#include "mem/bump_pool.hpp"

using namespace j;
using namespace j::lisp;
using namespace j::lisp::air;
using namespace j::lisp::air::exprs;
using j::mem::bump_pool;
using et = j::lisp::air::expr_type;

#define J_REQUIRE_UNARY_INFO(COND, ...) do { INFO(__VA_ARGS__); REQUIRE_UNARY(COND); } while (false)
#define J_REQUIRE_EQ_INFO(A, B, ...) do { INFO(__VA_ARGS__); REQUIRE_EQ(A, B); } while (false)

namespace j::lisp::air::inline passes {
  void trs_simplify(bump_pool & pool, expr * J_NOT_NULL root);
}

namespace {
  void check_inner(expr * e) {
    REQUIRE_LE(e->num_inputs, e->max_inputs);
    for (i32_t i = 0, max = e->num_inputs; i < max; ++i) {
      expr * child = e->in_expr(i);
      REQUIRE_UNARY(child);
      REQUIRE_UNARY(child->parent == e);
      REQUIRE_EQ(child->parent_pos, i);
      check_inner(child);
    }
  }

  void check(expr * e) {
    REQUIRE_UNARY(e);
    REQUIRE_UNARY(!e->parent);
    REQUIRE_EQ(e->type, et::fn_body);
    check_inner(e);
  }

  J_A(AI,MU) inline void assert_const(expr * J_NOT_NULL e, lisp_imm imm, const char * J_NOT_NULL path) {
    INFO("Expected ", path, " to eq ", imm);
    REQUIRE_EQ(e->type, et::ld_const);
    REQUIRE_UNARY(((ld_const*)e)->const_val.eq(imm));
  }

  J_A(AI,MU) inline void assert_const(expr * J_NOT_NULL e, u64_t imm, const char * J_NOT_NULL path) {
    assert_const(e, lisp_imm{imm}, path);
  }

  J_A(AI,MU,NODISC) inline input_span assert_inputs(input_span ins, u32_t size, const char * J_NOT_NULL path) {
    J_REQUIRE_EQ_INFO(ins.size(), size, "Expected ", path, " to have ", size, " elements");
    return ins;
  }

  template<typename Imm>
  J_A(AI,MU) inline void assert_sym_wr(expr * J_NOT_NULL e, u32_t sym, Imm imm, const char * J_NOT_NULL path, const char * J_NOT_NULL sub_path) {
    {
      INFO("Expected ", path, " to be '", lisp_imm((u64_t)sym));
      REQUIRE_EQ(e->type, et::sym_val_wr);
      REQUIRE_EQ(((sym_val_wr*)e)->var.raw, sym);
    }
    assert_const(((sym_val_wr*)e)->value.expr, imm, sub_path);
  }

  struct J_TYPE_HIDDEN trs_fixture {
    struct {
      bump_pool pool;
    } s;
    trs_writer w;
    env::environment e;
    env::env_ctx_guard g;

    J_A(RNN,NODISC) expr * apply_trs(expr * J_NOT_NULL in) {
      expr * body = write_expr(et::fn_body, s.pool, 0, span(&in, 1), allocate_vars(0));
      j::lisp::air::trs_simplify(s.pool, body);
      check(body);
      REQUIRE_EQ(body->num_inputs, 1);
      return body->in_expr(0);
    }

    trs_fixture()
      : w{s.pool},
        g(env::env_context.enter(&e)) {
      e.new_package("test");
      e.package_at("test")->emplace_id("a");
      e.package_at("test")->emplace_id("b");
      e.package_at("test")->emplace_id("c");
      e.package_at("test")->emplace_id("d");
    }
  };
}

TEST_SUITE_BEGIN("Lisp - AIR - TRS - If");

TEST_CASE_FIXTURE(trs_fixture, "(if true 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wrc(lisp_t), w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 2, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(if false 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wrc(lisp_f), w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(if nil 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wrc(lisp_imm{nil_v}), w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (progn) 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.progn(), w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (prog1) 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wr<>(et::prog1), w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (to_bool x) 1 2) -> (if x 1 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wr<>(et::to_bool, w.wr<id>(et::sym_val_rd, 1319)), w.i64(1), w.i64(2)));
  INFO(*result);
  b_if * const root = static_cast<b_if*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::b_if, "Expected root to be b_if.");
  sym_val_rd * const root_cond = static_cast<sym_val_rd*>(root->cond.expr);
  J_REQUIRE_EQ_INFO(root_cond->type, et::sym_val_rd, "Expected cond to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_cond->var).raw == 1319, "Expected cond.var to equal test:a");
  assert_const(root->then_branch.expr, 2, "then_branch");
  assert_const(root->else_branch.expr, 4, "else_branch");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (not x) 1 2) -> (if x 2 1)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wr<>(et::lnot, w.wr<id>(et::sym_val_rd, 1319)), w.i64(1), w.i64(2)));
  INFO(*result);
  b_if * const root = static_cast<b_if*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::b_if, "Expected root to be b_if.");
  sym_val_rd * const root_cond = static_cast<sym_val_rd*>(root->cond.expr);
  J_REQUIRE_EQ_INFO(root_cond->type, et::sym_val_rd, "Expected cond to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_cond->var).raw == 1319, "Expected cond.var to equal test:a");
  assert_const(root->then_branch.expr, 4, "then_branch");
  assert_const(root->else_branch.expr, 2, "else_branch");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (not (not x)) 1 2) -> (if x 1 3)") {
  expr * result = apply_trs(w.wr<>(et::b_if,
    w.wr<>(et::lnot, w.wr<>(et::lnot, w.wr<id>(et::sym_val_rd, 1319))),
    w.i64(1),
    w.i64(2)
  ));
  INFO(*result);
  b_if * const root = static_cast<b_if*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::b_if, "Expected root to be b_if.");
  sym_val_rd * const root_cond = static_cast<sym_val_rd*>(root->cond.expr);
  J_REQUIRE_EQ_INFO(root_cond->type, et::sym_val_rd, "Expected cond to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_cond->var).raw == 1319, "Expected cond.var to equal test:a");
  assert_const(root->then_branch.expr, 2, "then_branch");
  assert_const(root->else_branch.expr, 4, "else_branch");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (progn FX true) 1 2) -> (progn FX 1)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.progn(w.sym_wr(1319, w.i64(1)), w.wrc(lisp_t)), w.i64(1), w.i64(2)));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 2, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (progn FX false) 1 2) -> (progn FX 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.progn(w.sym_wr(1319, w.i64(1)), w.wrc(lisp_f)), w.i64(1), w.i64(2)));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 4, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (progn FX nil) 1 2) -> (progn FX 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if,
    w.progn(w.sym_wr(1319, w.i64(1)), w.wrc(lisp_imm{nil_v})),
    w.i64(1),
    w.i64(2)
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 4, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (prog1 true FX) 1 2) -> (progn FX 1)") {
  expr * result = apply_trs(w.wr<>(et::b_if,
    w.wr<>(et::prog1, w.wrc(lisp_t), w.sym_wr(1319, w.i64(1))),
    w.i64(1),
    w.i64(2)
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 2, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (prog1 false FX) 1 2) -> (progn FX 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if,
    w.wr<>(et::prog1, w.wrc(lisp_f), w.sym_wr(1319, w.i64(1))),
    w.i64(1),
    w.i64(2)
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 4, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if (prog1 nil FX) 1 2) -> (progn FX 2)") {
  expr * result = apply_trs(w.wr<>(et::b_if,
    w.wr<>(et::prog1, w.wrc(lisp_imm{nil_v}), w.sym_wr(1319, w.i64(1))),
    w.i64(1),
    w.i64(2)
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, 4, "forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(if x true false) -> (to_bool x)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wr<id>(et::sym_val_rd, 1319), w.wrc(lisp_t), w.wrc(lisp_f)));
  INFO(*result);
  to_bool * const root = static_cast<to_bool*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::to_bool, "Expected root to be to_bool.");
  sym_val_rd * const root_value = static_cast<sym_val_rd*>(root->value.expr);
  J_REQUIRE_EQ_INFO(root_value->type, et::sym_val_rd, "Expected value to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_value->var).raw == 1319, "Expected value.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(if x false true) -> (lnot x)") {
  expr * result = apply_trs(w.wr<>(et::b_if, w.wr<id>(et::sym_val_rd, 1319), w.wrc(lisp_f), w.wrc(lisp_t)));
  INFO(*result);
  lnot * const root = static_cast<lnot*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::lnot, "Expected root to be lnot.");
  sym_val_rd * const root_value = static_cast<sym_val_rd*>(root->value.expr);
  J_REQUIRE_EQ_INFO(root_value->type, et::sym_val_rd, "Expected value to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_value->var).raw == 1319, "Expected value.var to equal test:a");
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("Lisp - AIR - TRS - Vec");

TEST_CASE_FIXTURE(trs_fixture, "(vec)") {
  expr * result = apply_trs(w.wr<>(et::vec));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(mk-vec 0)") {
  expr * result = apply_trs(w.wr<>(et::make_vec, w.i64(0)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(append)") {
  expr * result = apply_trs(w.wr<>(et::vec_append));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(sym-rd '%air:*empty-vec*)") {
  expr * result = apply_trs(w.wr<id>(et::sym_val_rd, 66343));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-copy (vec))") {
  expr * result = apply_trs(w.wr<>(et::range_copy, w.wr<>(et::vec)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-copy (mk-vec 0))") {
  expr * result = apply_trs(w.wr<>(et::range_copy, w.wr<>(et::make_vec, w.i64(0))));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-copy (append))") {
  expr * result = apply_trs(w.wr<>(et::range_copy, w.wr<>(et::vec_append)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-copy (sym-rd '%air:*empty-vec*))") {
  expr * result = apply_trs(w.wr<>(et::range_copy, w.wr<id>(et::sym_val_rd, 66343)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (vec))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::vec)));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (vec 1))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::vec, w.i64(1))));
  INFO(*result);
  assert_const(result, 2, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (vec 1 0))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::vec, w.i64(1), w.i64(0))));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (mk-vec 0))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::make_vec, w.i64(0))));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (mk-vec 1))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::make_vec, w.i64(1))));
  INFO(*result);
  assert_const(result, 2, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (append))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::vec_append)));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (sym-rd '%air:*empty-vec*))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<id>(et::sym_val_rd, 66343)));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (rng-copy (vec)))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::range_copy, w.wr<>(et::vec))));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (rng-copy (mk-vec 0)))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::range_copy, w.wr<>(et::make_vec, w.i64(0)))));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (rng-copy (append)))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::range_copy, w.wr<>(et::vec_append))));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(rng-length (rng-copy (sym-rd '%air:*empty-vec*)))") {
  expr * result = apply_trs(w.wr<>(et::range_length, w.wr<>(et::range_copy, w.wr<id>(et::sym_val_rd, 66343))));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (append a b) c)") {
  expr * result = apply_trs(w.wr<>(et::vec_append,
    w.wr<>(et::vec_append, w.wr<id>(et::sym_val_rd, 1319), w.wr<id>(et::sym_val_rd, 66855)),
    w.wr<id>(et::sym_val_rd, 132391)
  ));
  INFO(*result);
  vec_append * const root = static_cast<vec_append*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec_append, "Expected root to be vec_append.");
  input_span root_vecs = assert_inputs(root->vecs(), 3, "vecs");
  sym_val_rd * const root_vecs_0 = static_cast<sym_val_rd*>((root_vecs)[0].expr);
  J_REQUIRE_EQ_INFO(root_vecs_0->type, et::sym_val_rd, "Expected vecs[0] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_0->var).raw == 1319, "Expected vecs[0].var to equal test:a");
  sym_val_rd * const root_vecs_1 = static_cast<sym_val_rd*>((root_vecs)[1].expr);
  J_REQUIRE_EQ_INFO(root_vecs_1->type, et::sym_val_rd, "Expected vecs[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_1->var).raw == 66855, "Expected vecs[1].var to equal test:b");
  sym_val_rd * const root_vecs_2 = static_cast<sym_val_rd*>((root_vecs)[2].expr);
  J_REQUIRE_EQ_INFO(root_vecs_2->type, et::sym_val_rd, "Expected vecs[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_2->var).raw == 132391, "Expected vecs[2].var to equal test:c");
}

TEST_CASE_FIXTURE(trs_fixture, "(append a (append b c))") {
  expr * result = apply_trs(w.wr<>(et::vec_append,
    w.wr<id>(et::sym_val_rd, 1319),
    w.wr<>(et::vec_append, w.wr<id>(et::sym_val_rd, 66855), w.wr<id>(et::sym_val_rd, 132391))
  ));
  INFO(*result);
  vec_append * const root = static_cast<vec_append*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec_append, "Expected root to be vec_append.");
  input_span root_vecs = assert_inputs(root->vecs(), 3, "vecs");
  sym_val_rd * const root_vecs_0 = static_cast<sym_val_rd*>((root_vecs)[0].expr);
  J_REQUIRE_EQ_INFO(root_vecs_0->type, et::sym_val_rd, "Expected vecs[0] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_0->var).raw == 1319, "Expected vecs[0].var to equal test:a");
  sym_val_rd * const root_vecs_1 = static_cast<sym_val_rd*>((root_vecs)[1].expr);
  J_REQUIRE_EQ_INFO(root_vecs_1->type, et::sym_val_rd, "Expected vecs[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_1->var).raw == 66855, "Expected vecs[1].var to equal test:b");
  sym_val_rd * const root_vecs_2 = static_cast<sym_val_rd*>((root_vecs)[2].expr);
  J_REQUIRE_EQ_INFO(root_vecs_2->type, et::sym_val_rd, "Expected vecs[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_2->var).raw == 132391, "Expected vecs[2].var to equal test:c");
}

TEST_CASE_FIXTURE(trs_fixture, "(append a (append b (append c d)))") {
  expr * result = apply_trs(w.wr<>(et::vec_append,
    w.wr<id>(et::sym_val_rd, 1319),
    w.wr<>(et::vec_append,
      w.wr<id>(et::sym_val_rd, 66855),
      w.wr<>(et::vec_append, w.wr<id>(et::sym_val_rd, 132391), w.wr<id>(et::sym_val_rd, 197927))
    )
  ));
  INFO(*result);
  vec_append * const root = static_cast<vec_append*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec_append, "Expected root to be vec_append.");
  input_span root_vecs = assert_inputs(root->vecs(), 4, "vecs");
  sym_val_rd * const root_vecs_0 = static_cast<sym_val_rd*>((root_vecs)[0].expr);
  J_REQUIRE_EQ_INFO(root_vecs_0->type, et::sym_val_rd, "Expected vecs[0] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_0->var).raw == 1319, "Expected vecs[0].var to equal test:a");
  sym_val_rd * const root_vecs_1 = static_cast<sym_val_rd*>((root_vecs)[1].expr);
  J_REQUIRE_EQ_INFO(root_vecs_1->type, et::sym_val_rd, "Expected vecs[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_1->var).raw == 66855, "Expected vecs[1].var to equal test:b");
  sym_val_rd * const root_vecs_2 = static_cast<sym_val_rd*>((root_vecs)[2].expr);
  J_REQUIRE_EQ_INFO(root_vecs_2->type, et::sym_val_rd, "Expected vecs[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_2->var).raw == 132391, "Expected vecs[2].var to equal test:c");
  sym_val_rd * const root_vecs_3 = static_cast<sym_val_rd*>((root_vecs)[3].expr);
  J_REQUIRE_EQ_INFO(root_vecs_3->type, et::sym_val_rd, "Expected vecs[3] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_3->var).raw == 197927, "Expected vecs[3].var to equal test:d");
}

TEST_CASE_FIXTURE(trs_fixture, "(append a (mk-vec 0) b)") {
  expr * result = apply_trs(w.wr<>(et::vec_append,
    w.wr<id>(et::sym_val_rd, 1319),
    w.wr<>(et::make_vec, w.i64(0)),
    w.wr<id>(et::sym_val_rd, 66855)
  ));
  INFO(*result);
  vec_append * const root = static_cast<vec_append*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec_append, "Expected root to be vec_append.");
  input_span root_vecs = assert_inputs(root->vecs(), 2, "vecs");
  sym_val_rd * const root_vecs_0 = static_cast<sym_val_rd*>((root_vecs)[0].expr);
  J_REQUIRE_EQ_INFO(root_vecs_0->type, et::sym_val_rd, "Expected vecs[0] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_0->var).raw == 1319, "Expected vecs[0].var to equal test:a");
  sym_val_rd * const root_vecs_1 = static_cast<sym_val_rd*>((root_vecs)[1].expr);
  J_REQUIRE_EQ_INFO(root_vecs_1->type, et::sym_val_rd, "Expected vecs[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_vecs_1->var).raw == 66855, "Expected vecs[1].var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (mk-vec 0) (mk-vec 0))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::make_vec, w.i64(0)), w.wr<>(et::make_vec, w.i64(0))));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (mk-vec 0) (mk-vec 1))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::make_vec, w.i64(0)), w.wr<>(et::make_vec, w.i64(1))));
  INFO(*result);
  make_vec * const root = static_cast<make_vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::make_vec, "Expected root to be make_vec.");
  assert_const(root->size.expr, 2, "size");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (mk-vec 1) (mk-vec 0))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::make_vec, w.i64(1)), w.wr<>(et::make_vec, w.i64(0))));
  INFO(*result);
  make_vec * const root = static_cast<make_vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::make_vec, "Expected root to be make_vec.");
  assert_const(root->size.expr, 2, "size");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (mk-vec 1) (mk-vec 2))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::make_vec, w.i64(1)), w.wr<>(et::make_vec, w.i64(1))));
  INFO(*result);
  vec_append * const root = static_cast<vec_append*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec_append, "Expected root to be vec_append.");
  input_span root_vecs = assert_inputs(root->vecs(), 2, "vecs");
  make_vec * const root_vecs_0 = static_cast<make_vec*>((root_vecs)[0].expr);
  J_REQUIRE_EQ_INFO(root_vecs_0->type, et::make_vec, "Expected vecs[0] to be make_vec.");
  assert_const(root_vecs_0->size.expr, 2, "vecs[0].size");
  make_vec * const root_vecs_1 = static_cast<make_vec*>((root_vecs)[1].expr);
  J_REQUIRE_EQ_INFO(root_vecs_1->type, et::make_vec, "Expected vecs[1] to be make_vec.");
  assert_const(root_vecs_1->size.expr, 2, "vecs[1].size");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (vec 1) (vec 2))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::vec, w.i64(1)), w.wr<>(et::vec, w.i64(2))));
  INFO(*result);
  vec * const root = static_cast<vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec, "Expected root to be vec.");
  input_span root_elems = assert_inputs(root->elems(), 2, "elems");
  assert_const((root_elems)[0].expr, 2, "elems[0]");
  assert_const((root_elems)[1].expr, 4, "elems[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (vec) (vec 2))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::vec), w.wr<>(et::vec, w.i64(2))));
  INFO(*result);
  vec * const root = static_cast<vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec, "Expected root to be vec.");
  assert_const(assert_inputs(root->elems(), 1, "elems")[0].expr, 4, "elems[0]");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (vec 1) (vec) (vec 2))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::vec, w.i64(1)), w.wr<>(et::vec), w.wr<>(et::vec, w.i64(2))));
  INFO(*result);
  vec * const root = static_cast<vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::vec, "Expected root to be vec.");
  input_span root_elems = assert_inputs(root->elems(), 2, "elems");
  assert_const((root_elems)[0].expr, 2, "elems[0]");
  assert_const((root_elems)[1].expr, 4, "elems[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(append (vec) (vec))") {
  expr * result = apply_trs(w.wr<>(et::vec_append, w.wr<>(et::vec), w.wr<>(et::vec)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 66343, "Expected root.var to equal %air:*empty-vec*");
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("Lisp - AIR - TRS - Progn");

TEST_CASE_FIXTURE(trs_fixture, "Empty progn") {
  expr * result = apply_trs(w.progn());
  INFO(*result);
  assert_const(result, lisp_imm{nil_v}, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "Single-valued progn") {
  expr * result = apply_trs(w.progn(w.i64(1)));
  INFO(*result);
  assert_const(result, 2, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn NOFX FX)") {
  expr * result = apply_trs(w.progn(w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 4, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn (progn FX FX) (progn FX FX))") {
  expr * result = apply_trs(w.progn(
    w.progn(w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2))),
    w.progn(w.sym_wr(132391, w.i64(3)), w.sym_wr(197927, w.i64(4)))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 4, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_sym_wr((root_forms)[2].expr, 132391, 6, "forms[2]", "forms[2].value");
  assert_sym_wr((root_forms)[3].expr, 197927, 8, "forms[3]", "forms[3].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn (progn FX 1) (progn FX 2))") {
  expr * result = apply_trs(w.progn(
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(2)),
    w.progn(w.sym_wr(132391, w.i64(3)), w.i64(4))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 132391, 6, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 8, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn 1 (progn 2) 3) -> (progn 1 2 3)") {
  expr * result = apply_trs(w.progn(
    w.sym_wr(1319, w.i64(1)),
    w.progn(w.sym_wr(66855, w.i64(2))),
    w.sym_wr(132391, w.i64(3))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_sym_wr((root_forms)[2].expr, 132391, 6, "forms[2]", "forms[2].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn FX (progn FX NOFX))") {
  expr * result = apply_trs(w.progn(w.sym_wr(1319, w.i64(1)), w.progn(w.sym_wr(66855, w.i64(2)), w.i64(3))));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 6, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(progn FX a FX b)") {
  expr * result = apply_trs(w.progn(
    w.i64(1),
    w.sym_wr(1319, w.i64(1)),
    w.wr<id>(et::sym_val_rd, 1319),
    w.sym_wr(66855, w.i64(2)),
    w.wr<id>(et::sym_val_rd, 66855)
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  sym_val_rd * const root_forms_2 = static_cast<sym_val_rd*>((root_forms)[2].expr);
  J_REQUIRE_EQ_INFO(root_forms_2->type, et::sym_val_rd, "Expected forms[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_2->var).raw == 66855, "Expected forms[2].var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(prog1 r1 (progn FX NOFX))") {
  expr * result = apply_trs(w.wr<>(et::prog1,
    w.wr<id>(et::sym_val_rd, 1319),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(3))
  ));
  INFO(*result);
  prog1 * const root = static_cast<prog1*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::prog1, "Expected root to be prog1.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  sym_val_rd * const root_forms_0 = static_cast<sym_val_rd*>((root_forms)[0].expr);
  J_REQUIRE_EQ_INFO(root_forms_0->type, et::sym_val_rd, "Expected forms[0] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_0->var).raw == 1319, "Expected forms[0].var to equal test:a");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(prog1 false w1) -> (progn w1 false)") {
  expr * result = apply_trs(w.wr<>(et::prog1, w.wrc(lisp_f), w.sym_wr(1319, w.i64(1))));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_const((root_forms)[1].expr, lisp_f, "forms[1]");
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("Lisp - AIR - TRS - IAdd");

TEST_CASE_FIXTURE(trs_fixture, "(+ a b)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 1319), w.wr<id>(et::sym_val_rd, 66855)));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  sym_val_rd * const root_rhs = static_cast<sym_val_rd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::sym_val_rd, "Expected rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs->var).raw == 66855, "Expected rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 1 2)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, 6, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ a 1)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 1319), w.i64(1)));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 2, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 1 a)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 2, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ a 0)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 1319), w.i64(0)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 1319, "Expected root.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 0 a)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(0), w.wr<id>(et::sym_val_rd, 1319)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 1319, "Expected root.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 0 0)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(0), w.i64(0)));
  INFO(*result);
  assert_const(result, 0, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 1) (progn w2 1))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(1)),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(1))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 4, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 1) (progn w2 0))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(1)),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(0))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 2, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 0) (progn w2 1))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(0)),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(1))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 2, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 0) (progn w2 0))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(0)),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(0))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 0, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 (+ 1 a)) (progn w2 (+ 2 b)))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(
      w.sym_wr(1319, w.i64(1)),
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319))
    ),
    w.progn(
      w.sym_wr(66855, w.i64(2)),
      w.wr<>(et::iadd, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
    )
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  iadd * const root_forms_1 = static_cast<iadd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::iadd, "Expected forms[1] to be iadd.");
  iadd * const root_forms_1_lhs = static_cast<iadd*>(root_forms_1->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs->type, et::iadd, "Expected forms[1].lhs to be iadd.");
  sym_val_rd * const root_forms_1_lhs_lhs = static_cast<sym_val_rd*>(root_forms_1_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_lhs->type, et::sym_val_rd, "Expected forms[1].lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_lhs->var).raw == 1319, "Expected forms[1].lhs.lhs.var to equal test:a");
  progn * const root_forms_1_lhs_rhs = static_cast<progn*>(root_forms_1_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_rhs->type, et::progn, "Expected forms[1].lhs.rhs to be progn.");
  input_span root_forms_1_lhs_rhs_forms = assert_inputs(root_forms_1_lhs_rhs->forms(), 2, "forms[1].lhs.rhs.forms");
  assert_sym_wr((root_forms_1_lhs_rhs_forms)[0].expr, 66855, 4, "forms[1].lhs.rhs.forms[0]", "forms[1].lhs.rhs.forms[0].value");
  sym_val_rd * const root_forms_1_lhs_rhs_forms_1 = static_cast<sym_val_rd*>((root_forms_1_lhs_rhs_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_rhs_forms_1->type, et::sym_val_rd, "Expected forms[1].lhs.rhs.forms[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_rhs_forms_1->var).raw == 66855, "Expected forms[1].lhs.rhs.forms[1].var to equal test:b");
  assert_const(root_forms_1->rhs.expr, 6, "forms[1].rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 (+ 1 a)) (progn w2 0))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(
      w.sym_wr(1319, w.i64(1)),
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319))
    ),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(0))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  iadd * const root_forms_1 = static_cast<iadd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::iadd, "Expected forms[1] to be iadd.");
  sym_val_rd * const root_forms_1_lhs = static_cast<sym_val_rd*>(root_forms_1->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs->type, et::sym_val_rd, "Expected forms[1].lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs->var).raw == 1319, "Expected forms[1].lhs.var to equal test:a");
  progn * const root_forms_1_rhs = static_cast<progn*>(root_forms_1->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_rhs->type, et::progn, "Expected forms[1].rhs to be progn.");
  input_span root_forms_1_rhs_forms = assert_inputs(root_forms_1_rhs->forms(), 2, "forms[1].rhs.forms");
  assert_sym_wr((root_forms_1_rhs_forms)[0].expr, 66855, 4, "forms[1].rhs.forms[0]", "forms[1].rhs.forms[0].value");
  assert_const((root_forms_1_rhs_forms)[1].expr, 2, "forms[1].rhs.forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 0) (progn w2 (+ 1 a)))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(0)),
    w.progn(
      w.sym_wr(66855, w.i64(2)),
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319))
    )
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  iadd * const root_forms_2 = static_cast<iadd*>((root_forms)[2].expr);
  J_REQUIRE_EQ_INFO(root_forms_2->type, et::iadd, "Expected forms[2] to be iadd.");
  sym_val_rd * const root_forms_2_lhs = static_cast<sym_val_rd*>(root_forms_2->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_2_lhs->type, et::sym_val_rd, "Expected forms[2].lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_2_lhs->var).raw == 1319, "Expected forms[2].lhs.var to equal test:a");
  assert_const(root_forms_2->rhs.expr, 2, "forms[2].rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- a 1)") {
  expr * result = apply_trs(w.wr<>(et::isub, w.wr<id>(et::sym_val_rd, 1319), w.i64(1)));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 2, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ a -1)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 1319), w.i64(-1)));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 2, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- a 0)") {
  expr * result = apply_trs(w.wr<>(et::isub, w.wr<id>(et::sym_val_rd, 1319), w.i64(0)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 1319, "Expected root.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 1 (+ 2 a))") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(1), w.wr<>(et::iadd, w.i64(2), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 6, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 1 a) 2)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)), w.i64(2)));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 6, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 1 (- 2 a))") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(1), w.wr<>(et::isub, w.i64(2), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 6, "lhs");
  sym_val_rd * const root_rhs = static_cast<sym_val_rd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::sym_val_rd, "Expected rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs->var).raw == 1319, "Expected rhs.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (- 1 a) 2)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<>(et::isub, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)), w.i64(2)));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 6, "lhs");
  sym_val_rd * const root_rhs = static_cast<sym_val_rd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::sym_val_rd, "Expected rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs->var).raw == 1319, "Expected rhs.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ 1 (+ -1 a))") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.i64(1), w.wr<>(et::iadd, w.i64(-1), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 1319, "Expected root.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 1 a) -1)") {
  expr * result = apply_trs(w.wr<>(et::iadd, w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)), w.i64(-1)));
  INFO(*result);
  sym_val_rd * const root = static_cast<sym_val_rd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::sym_val_rd, "Expected root to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root->var).raw == 1319, "Expected root.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 1 a) (+ 2 b))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::iadd, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 66855, "Expected lhs.rhs.var to equal test:b");
  assert_const(root->rhs.expr, 6, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 6 a) (+ -6 b))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::iadd, w.i64(6), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::iadd, w.i64(-6), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  sym_val_rd * const root_rhs = static_cast<sym_val_rd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::sym_val_rd, "Expected rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs->var).raw == 66855, "Expected rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 7 fx1) (+ 6 fx2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::iadd, w.i64(6), w.wr<>(et::act_rec_parent, w.wr<id>(et::sym_val_rd, 1319))),
    w.wr<>(et::iadd, w.i64(7), w.wr<>(et::act_rec_parent, w.wr<id>(et::sym_val_rd, 66855)))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  act_rec_parent * const root_lhs_lhs = static_cast<act_rec_parent*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::act_rec_parent, "Expected lhs.lhs to be act_rec_parent.");
  sym_val_rd * const root_lhs_lhs_act_rec = static_cast<sym_val_rd*>(root_lhs_lhs->act_rec.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs_act_rec->type, et::sym_val_rd, "Expected lhs.lhs.act_rec to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs_act_rec->var).raw == 1319, "Expected lhs.lhs.act_rec.var to equal test:a");
  act_rec_parent * const root_lhs_rhs = static_cast<act_rec_parent*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::act_rec_parent, "Expected lhs.rhs to be act_rec_parent.");
  sym_val_rd * const root_lhs_rhs_act_rec = static_cast<sym_val_rd*>(root_lhs_rhs->act_rec.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs_act_rec->type, et::sym_val_rd, "Expected lhs.rhs.act_rec to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs_act_rec->var).raw == 66855, "Expected lhs.rhs.act_rec.var to equal test:b");
  assert_const(root->rhs.expr, 26, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 7 fx1) (+ -7 fx2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::iadd, w.i64(7), w.wr<>(et::act_rec_parent, w.wr<id>(et::sym_val_rd, 1319))),
    w.wr<>(et::iadd, w.i64(-7), w.wr<>(et::act_rec_parent, w.wr<id>(et::sym_val_rd, 66855)))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  act_rec_parent * const root_lhs = static_cast<act_rec_parent*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::act_rec_parent, "Expected lhs to be act_rec_parent.");
  sym_val_rd * const root_lhs_act_rec = static_cast<sym_val_rd*>(root_lhs->act_rec.expr);
  J_REQUIRE_EQ_INFO(root_lhs_act_rec->type, et::sym_val_rd, "Expected lhs.act_rec to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_act_rec->var).raw == 1319, "Expected lhs.act_rec.var to equal test:a");
  act_rec_parent * const root_rhs = static_cast<act_rec_parent*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::act_rec_parent, "Expected rhs to be act_rec_parent.");
  sym_val_rd * const root_rhs_act_rec = static_cast<sym_val_rd*>(root_rhs->act_rec.expr);
  J_REQUIRE_EQ_INFO(root_rhs_act_rec->type, et::sym_val_rd, "Expected rhs.act_rec to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_act_rec->var).raw == 66855, "Expected rhs.act_rec.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 1) (progn w2 2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(1)),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(2))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 6, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (prog1 1 w1) (prog1 2 w2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::prog1, w.i64(1), w.sym_wr(1319, w.i64(1))),
    w.wr<>(et::prog1, w.i64(2), w.sym_wr(66855, w.i64(2)))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 6, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (prog1 1 w1) (progn w2 2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::prog1, w.i64(1), w.sym_wr(1319, w.i64(1))),
    w.progn(w.sym_wr(66855, w.i64(2)), w.i64(2))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 6, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 1) (prog1 2 w2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(w.sym_wr(1319, w.i64(1)), w.i64(1)),
    w.wr<>(et::prog1, w.i64(2), w.sym_wr(66855, w.i64(2)))
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 3, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  assert_sym_wr((root_forms)[1].expr, 66855, 4, "forms[1]", "forms[1].value");
  assert_const((root_forms)[2].expr, 6, "forms[2]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 (+ 1 r1)) (prog1 (+ r2 2) w2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(
      w.sym_wr(1319, w.i64(1)),
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319))
    ),
    w.wr<>(et::prog1,
      w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 66855), w.i64(2)),
      w.sym_wr(66855, w.i64(2))
    )
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  iadd * const root_forms_1 = static_cast<iadd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::iadd, "Expected forms[1] to be iadd.");
  iadd * const root_forms_1_lhs = static_cast<iadd*>(root_forms_1->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs->type, et::iadd, "Expected forms[1].lhs to be iadd.");
  sym_val_rd * const root_forms_1_lhs_lhs = static_cast<sym_val_rd*>(root_forms_1_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_lhs->type, et::sym_val_rd, "Expected forms[1].lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_lhs->var).raw == 1319, "Expected forms[1].lhs.lhs.var to equal test:a");
  sym_val_rd * const root_forms_1_lhs_rhs = static_cast<sym_val_rd*>(root_forms_1_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_rhs->type, et::sym_val_rd, "Expected forms[1].lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_rhs->var).raw == 66855, "Expected forms[1].lhs.rhs.var to equal test:b");
  progn * const root_forms_1_rhs = static_cast<progn*>(root_forms_1->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_rhs->type, et::progn, "Expected forms[1].rhs to be progn.");
  input_span root_forms_1_rhs_forms = assert_inputs(root_forms_1_rhs->forms(), 2, "forms[1].rhs.forms");
  assert_sym_wr((root_forms_1_rhs_forms)[0].expr, 66855, 4, "forms[1].rhs.forms[0]", "forms[1].rhs.forms[0].value");
  assert_const((root_forms_1_rhs_forms)[1].expr, 6, "forms[1].rhs.forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (prog1 (+ 1 r1) w1) (prog1 (+ r2 2) w2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::prog1,
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)),
      w.sym_wr(1319, w.i64(1))
    ),
    w.wr<>(et::prog1,
      w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 66855), w.i64(2)),
      w.sym_wr(66855, w.i64(2))
    )
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  progn * const root_lhs_rhs = static_cast<progn*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::progn, "Expected lhs.rhs to be progn.");
  input_span root_lhs_rhs_forms = assert_inputs(root_lhs_rhs->forms(), 2, "lhs.rhs.forms");
  assert_sym_wr((root_lhs_rhs_forms)[0].expr, 1319, 2, "lhs.rhs.forms[0]", "lhs.rhs.forms[0].value");
  sym_val_rd * const root_lhs_rhs_forms_1 = static_cast<sym_val_rd*>((root_lhs_rhs_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs_forms_1->type, et::sym_val_rd, "Expected lhs.rhs.forms[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs_forms_1->var).raw == 66855, "Expected lhs.rhs.forms[1].var to equal test:b");
  progn * const root_rhs = static_cast<progn*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::progn, "Expected rhs to be progn.");
  input_span root_rhs_forms = assert_inputs(root_rhs->forms(), 2, "rhs.forms");
  assert_sym_wr((root_rhs_forms)[0].expr, 66855, 4, "rhs.forms[0]", "rhs.forms[0].value");
  assert_const((root_rhs_forms)[1].expr, 6, "rhs.forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (prog1 (+ 1 r1) w1) (progn w2 (+ r2 2)))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::prog1,
      w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)),
      w.sym_wr(1319, w.i64(1))
    ),
    w.progn(
      w.sym_wr(66855, w.i64(2)),
      w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 66855), w.i64(2))
    )
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  progn * const root_lhs_rhs = static_cast<progn*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::progn, "Expected lhs.rhs to be progn.");
  input_span root_lhs_rhs_forms = assert_inputs(root_lhs_rhs->forms(), 3, "lhs.rhs.forms");
  assert_sym_wr((root_lhs_rhs_forms)[0].expr, 1319, 2, "lhs.rhs.forms[0]", "lhs.rhs.forms[0].value");
  assert_sym_wr((root_lhs_rhs_forms)[1].expr, 66855, 4, "lhs.rhs.forms[1]", "lhs.rhs.forms[1].value");
  sym_val_rd * const root_lhs_rhs_forms_2 = static_cast<sym_val_rd*>((root_lhs_rhs_forms)[2].expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs_forms_2->type, et::sym_val_rd, "Expected lhs.rhs.forms[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs_forms_2->var).raw == 66855, "Expected lhs.rhs.forms[2].var to equal test:b");
  assert_const(root->rhs.expr, 6, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (progn w1 (prog1 (+ 1 r1) w2)) (progn w3 (prog1 (+ r2 2) w4)))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.progn(
      w.sym_wr(1319, w.i64(1)),
      w.wr<>(et::prog1,
        w.wr<>(et::iadd, w.i64(1), w.wr<id>(et::sym_val_rd, 1319)),
        w.sym_wr(66855, w.i64(2))
      )
    ),
    w.progn(
      w.sym_wr(132391, w.i64(3)),
      w.wr<>(et::prog1,
        w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 66855), w.i64(2)),
        w.sym_wr(197927, w.i64(4))
      )
    )
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  iadd * const root_forms_1 = static_cast<iadd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::iadd, "Expected forms[1] to be iadd.");
  iadd * const root_forms_1_lhs = static_cast<iadd*>(root_forms_1->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs->type, et::iadd, "Expected forms[1].lhs to be iadd.");
  sym_val_rd * const root_forms_1_lhs_lhs = static_cast<sym_val_rd*>(root_forms_1_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_lhs->type, et::sym_val_rd, "Expected forms[1].lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_lhs->var).raw == 1319, "Expected forms[1].lhs.lhs.var to equal test:a");
  progn * const root_forms_1_lhs_rhs = static_cast<progn*>(root_forms_1_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_rhs->type, et::progn, "Expected forms[1].lhs.rhs to be progn.");
  input_span root_forms_1_lhs_rhs_forms = assert_inputs(root_forms_1_lhs_rhs->forms(), 3, "forms[1].lhs.rhs.forms");
  assert_sym_wr((root_forms_1_lhs_rhs_forms)[0].expr, 66855, 4, "forms[1].lhs.rhs.forms[0]", "forms[1].lhs.rhs.forms[0].value");
  assert_sym_wr((root_forms_1_lhs_rhs_forms)[1].expr, 132391, 6, "forms[1].lhs.rhs.forms[1]", "forms[1].lhs.rhs.forms[1].value");
  sym_val_rd * const root_forms_1_lhs_rhs_forms_2 = static_cast<sym_val_rd*>((root_forms_1_lhs_rhs_forms)[2].expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs_rhs_forms_2->type, et::sym_val_rd, "Expected forms[1].lhs.rhs.forms[2] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs_rhs_forms_2->var).raw == 66855, "Expected forms[1].lhs.rhs.forms[2].var to equal test:b");
  progn * const root_forms_1_rhs = static_cast<progn*>(root_forms_1->rhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_rhs->type, et::progn, "Expected forms[1].rhs to be progn.");
  input_span root_forms_1_rhs_forms = assert_inputs(root_forms_1_rhs->forms(), 2, "forms[1].rhs.forms");
  assert_sym_wr((root_forms_1_rhs_forms)[0].expr, 197927, 8, "forms[1].rhs.forms[0]", "forms[1].rhs.forms[0].value");
  assert_const((root_forms_1_rhs_forms)[1].expr, 6, "forms[1].rhs.forms[1]");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (+ 7 r1))") {
  expr * result = apply_trs(w.wr<>(et::ineg, w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  ineg * const root = static_cast<ineg*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::ineg, "Expected root to be ineg.");
  iadd * const root_value = static_cast<iadd*>(root->value.expr);
  J_REQUIRE_EQ_INFO(root_value->type, et::iadd, "Expected value to be iadd.");
  sym_val_rd * const root_value_lhs = static_cast<sym_val_rd*>(root_value->lhs.expr);
  J_REQUIRE_EQ_INFO(root_value_lhs->type, et::sym_val_rd, "Expected value.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_value_lhs->var).raw == 1319, "Expected value.lhs.var to equal test:a");
  assert_const(root_value->rhs.expr, 14, "value.rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (- 7 r1))") {
  expr * result = apply_trs(w.wr<>(et::ineg, w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (p w1 (- (+ 7 r1))))") {
  expr * result = apply_trs(w.wr<>(et::ineg,
    w.progn(
      w.sym_wr(1319, w.i64(1)),
      w.wr<>(et::ineg, w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)))
    )
  ));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  iadd * const root_forms_1 = static_cast<iadd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::iadd, "Expected forms[1] to be iadd.");
  sym_val_rd * const root_forms_1_lhs = static_cast<sym_val_rd*>(root_forms_1->lhs.expr);
  J_REQUIRE_EQ_INFO(root_forms_1_lhs->type, et::sym_val_rd, "Expected forms[1].lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1_lhs->var).raw == 1319, "Expected forms[1].lhs.var to equal test:a");
  assert_const(root_forms_1->rhs.expr, 14, "forms[1].rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- 9 (+ 7 r1))") {
  expr * result = apply_trs(w.wr<>(et::isub, w.i64(9), w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 4, "lhs");
  sym_val_rd * const root_rhs = static_cast<sym_val_rd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::sym_val_rd, "Expected rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs->var).raw == 1319, "Expected rhs.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (+ 7 r1) 9)") {
  expr * result = apply_trs(w.wr<>(et::isub, w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)), w.i64(9)));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  sym_val_rd * const root_lhs = static_cast<sym_val_rd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::sym_val_rd, "Expected lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs->var).raw == 1319, "Expected lhs.var to equal test:a");
  assert_const(root->rhs.expr, 4, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (- 7 r1) 9)") {
  expr * result = apply_trs(w.wr<>(et::isub, w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)), w.i64(9)));
  INFO(*result);
  ineg * const root = static_cast<ineg*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::ineg, "Expected root to be ineg.");
  iadd * const root_value = static_cast<iadd*>(root->value.expr);
  J_REQUIRE_EQ_INFO(root_value->type, et::iadd, "Expected value to be iadd.");
  sym_val_rd * const root_value_lhs = static_cast<sym_val_rd*>(root_value->lhs.expr);
  J_REQUIRE_EQ_INFO(root_value_lhs->type, et::sym_val_rd, "Expected value.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_value_lhs->var).raw == 1319, "Expected value.lhs.var to equal test:a");
  assert_const(root_value->rhs.expr, 4, "value.rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (+ 7 r1) (+ r2 2))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::iadd, w.wr<id>(et::sym_val_rd, 66855), w.i64(2))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  isub * const root_lhs = static_cast<isub*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::isub, "Expected lhs to be isub.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 66855, "Expected lhs.rhs.var to equal test:b");
  assert_const(root->rhs.expr, 10, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (- 7 r1) (- 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::isub, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 10, "lhs");
  isub * const root_rhs = static_cast<isub*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::isub, "Expected rhs to be isub.");
  sym_val_rd * const root_rhs_lhs = static_cast<sym_val_rd*>(root_rhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_lhs->type, et::sym_val_rd, "Expected rhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_lhs->var).raw == 1319, "Expected rhs.lhs.var to equal test:a");
  sym_val_rd * const root_rhs_rhs = static_cast<sym_val_rd*>(root_rhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_rhs->type, et::sym_val_rd, "Expected rhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_rhs->var).raw == 66855, "Expected rhs.rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (+ 7 r1) (- 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::isub, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 66855, "Expected lhs.rhs.var to equal test:b");
  assert_const(root->rhs.expr, 10, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (- 7 r1) (+ 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::iadd, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 10, "lhs");
  iadd * const root_rhs = static_cast<iadd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::iadd, "Expected rhs to be iadd.");
  sym_val_rd * const root_rhs_lhs = static_cast<sym_val_rd*>(root_rhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_lhs->type, et::sym_val_rd, "Expected rhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_lhs->var).raw == 1319, "Expected rhs.lhs.var to equal test:a");
  sym_val_rd * const root_rhs_rhs = static_cast<sym_val_rd*>(root_rhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_rhs->type, et::sym_val_rd, "Expected rhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_rhs->var).raw == 66855, "Expected rhs.rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (- 7 r1) (- 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::isub, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 18, "lhs");
  iadd * const root_rhs = static_cast<iadd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::iadd, "Expected rhs to be iadd.");
  sym_val_rd * const root_rhs_lhs = static_cast<sym_val_rd*>(root_rhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_lhs->type, et::sym_val_rd, "Expected rhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_lhs->var).raw == 1319, "Expected rhs.lhs.var to equal test:a");
  sym_val_rd * const root_rhs_rhs = static_cast<sym_val_rd*>(root_rhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_rhs->type, et::sym_val_rd, "Expected rhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_rhs->var).raw == 66855, "Expected rhs.rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (+ 7 r1) (- 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::isub, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  isub * const root_lhs = static_cast<isub*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::isub, "Expected lhs to be isub.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 66855, "Expected lhs.rhs.var to equal test:b");
  assert_const(root->rhs.expr, 18, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(+ (- 7 r1) (+ 2 r2))") {
  expr * result = apply_trs(w.wr<>(et::iadd,
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<>(et::iadd, w.i64(2), w.wr<id>(et::sym_val_rd, 66855))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 18, "lhs");
  isub * const root_rhs = static_cast<isub*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::isub, "Expected rhs to be isub.");
  sym_val_rd * const root_rhs_lhs = static_cast<sym_val_rd*>(root_rhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_lhs->type, et::sym_val_rd, "Expected rhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_lhs->var).raw == 1319, "Expected rhs.lhs.var to equal test:a");
  sym_val_rd * const root_rhs_rhs = static_cast<sym_val_rd*>(root_rhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_rhs->type, et::sym_val_rd, "Expected rhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_rhs->var).raw == 66855, "Expected rhs.rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (+ 7 r1) r2)") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<id>(et::sym_val_rd, 66855)
  ));
  INFO(*result);
  iadd * const root = static_cast<iadd*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::iadd, "Expected root to be iadd.");
  isub * const root_lhs = static_cast<isub*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::isub, "Expected lhs to be isub.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 1319, "Expected lhs.lhs.var to equal test:a");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 66855, "Expected lhs.rhs.var to equal test:b");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- (- 7 r1) r2)") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319)),
    w.wr<id>(et::sym_val_rd, 66855)
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  assert_const(root->lhs.expr, 14, "lhs");
  iadd * const root_rhs = static_cast<iadd*>(root->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs->type, et::iadd, "Expected rhs to be iadd.");
  sym_val_rd * const root_rhs_lhs = static_cast<sym_val_rd*>(root_rhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_lhs->type, et::sym_val_rd, "Expected rhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_lhs->var).raw == 1319, "Expected rhs.lhs.var to equal test:a");
  sym_val_rd * const root_rhs_rhs = static_cast<sym_val_rd*>(root_rhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_rhs_rhs->type, et::sym_val_rd, "Expected rhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_rhs_rhs->var).raw == 66855, "Expected rhs.rhs.var to equal test:b");
}

TEST_CASE_FIXTURE(trs_fixture, "(- r2 (+ 7 r1))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<id>(et::sym_val_rd, 66855),
    w.wr<>(et::iadd, w.i64(7), w.wr<id>(et::sym_val_rd, 1319))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  isub * const root_lhs = static_cast<isub*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::isub, "Expected lhs to be isub.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 66855, "Expected lhs.lhs.var to equal test:b");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 1319, "Expected lhs.rhs.var to equal test:a");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- r2 (- 7 r1))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<id>(et::sym_val_rd, 66855),
    w.wr<>(et::isub, w.i64(7), w.wr<id>(et::sym_val_rd, 1319))
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 66855, "Expected lhs.lhs.var to equal test:b");
  sym_val_rd * const root_lhs_rhs = static_cast<sym_val_rd*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::sym_val_rd, "Expected lhs.rhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs->var).raw == 1319, "Expected lhs.rhs.var to equal test:a");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- r2 (+ (progn w1 7) r1))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<id>(et::sym_val_rd, 66855),
    w.wr<>(et::iadd,
      w.progn(w.sym_wr(1319, w.i64(1)), w.i64(7)),
      w.wr<id>(et::sym_val_rd, 1319)
    )
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  isub * const root_lhs = static_cast<isub*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::isub, "Expected lhs to be isub.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 66855, "Expected lhs.lhs.var to equal test:b");
  progn * const root_lhs_rhs = static_cast<progn*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::progn, "Expected lhs.rhs to be progn.");
  input_span root_lhs_rhs_forms = assert_inputs(root_lhs_rhs->forms(), 2, "lhs.rhs.forms");
  assert_sym_wr((root_lhs_rhs_forms)[0].expr, 1319, 2, "lhs.rhs.forms[0]", "lhs.rhs.forms[0].value");
  sym_val_rd * const root_lhs_rhs_forms_1 = static_cast<sym_val_rd*>((root_lhs_rhs_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs_forms_1->type, et::sym_val_rd, "Expected lhs.rhs.forms[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs_forms_1->var).raw == 1319, "Expected lhs.rhs.forms[1].var to equal test:a");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- r2 (- (progn w1 7) r1))") {
  expr * result = apply_trs(w.wr<>(et::isub,
    w.wr<id>(et::sym_val_rd, 66855),
    w.wr<>(et::isub,
      w.progn(w.sym_wr(1319, w.i64(1)), w.i64(7)),
      w.wr<id>(et::sym_val_rd, 1319)
    )
  ));
  INFO(*result);
  isub * const root = static_cast<isub*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::isub, "Expected root to be isub.");
  iadd * const root_lhs = static_cast<iadd*>(root->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs->type, et::iadd, "Expected lhs to be iadd.");
  sym_val_rd * const root_lhs_lhs = static_cast<sym_val_rd*>(root_lhs->lhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_lhs->type, et::sym_val_rd, "Expected lhs.lhs to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_lhs->var).raw == 66855, "Expected lhs.lhs.var to equal test:b");
  progn * const root_lhs_rhs = static_cast<progn*>(root_lhs->rhs.expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs->type, et::progn, "Expected lhs.rhs to be progn.");
  input_span root_lhs_rhs_forms = assert_inputs(root_lhs_rhs->forms(), 2, "lhs.rhs.forms");
  assert_sym_wr((root_lhs_rhs_forms)[0].expr, 1319, 2, "lhs.rhs.forms[0]", "lhs.rhs.forms[0].value");
  sym_val_rd * const root_lhs_rhs_forms_1 = static_cast<sym_val_rd*>((root_lhs_rhs_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_lhs_rhs_forms_1->type, et::sym_val_rd, "Expected lhs.rhs.forms[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_lhs_rhs_forms_1->var).raw == 1319, "Expected lhs.rhs.forms[1].var to equal test:a");
  assert_const(root->rhs.expr, 14, "rhs");
}

TEST_CASE_FIXTURE(trs_fixture, "(- 1 2)") {
  expr * result = apply_trs(w.wr<>(et::isub, w.i64(1), w.i64(2)));
  INFO(*result);
  assert_const(result, -2, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(* 2 3)") {
  expr * result = apply_trs(w.wr<>(et::imul, w.i64(2), w.i64(3)));
  INFO(*result);
  assert_const(result, 12, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(/ 7 2)") {
  expr * result = apply_trs(w.wr<>(et::idiv, w.i64(7), w.i64(2)));
  INFO(*result);
  assert_const(result, 6, "root");
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("Lisp - AIR - TRS - Logic");

TEST_CASE_FIXTURE(trs_fixture, "(not false)") {
  expr * result = apply_trs(w.wr<>(et::land, w.wr<>(et::lnot, w.wrc(lisp_f))));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(not true)") {
  expr * result = apply_trs(w.wr<>(et::land, w.wr<>(et::lnot, w.wrc(lisp_t))));
  INFO(*result);
  assert_const(result, lisp_f, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and)") {
  expr * result = apply_trs(w.wr<>(et::land));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or)") {
  expr * result = apply_trs(w.wr<>(et::lor));
  INFO(*result);
  assert_const(result, lisp_f, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and true)") {
  expr * result = apply_trs(w.wr<>(et::land, w.wrc(lisp_t)));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or true)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.wrc(lisp_t)));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and false)") {
  expr * result = apply_trs(w.wr<>(et::land, w.wrc(lisp_f)));
  INFO(*result);
  assert_const(result, lisp_f, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or false)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.wrc(lisp_f)));
  INFO(*result);
  assert_const(result, lisp_f, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and nil)") {
  expr * result = apply_trs(w.wr<>(et::land, w.wrc(lisp_imm{nil_v})));
  INFO(*result);
  assert_const(result, lisp_imm{nil_v}, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or nil)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.wrc(lisp_imm{nil_v})));
  INFO(*result);
  assert_const(result, lisp_imm{nil_v}, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and 11)") {
  expr * result = apply_trs(w.wr<>(et::land, w.i64(11)));
  INFO(*result);
  assert_const(result, 22, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or 11)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.i64(11)));
  INFO(*result);
  assert_const(result, 22, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or false 11)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.wrc(lisp_f), w.i64(11)));
  INFO(*result);
  assert_const(result, 22, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or True 11)") {
  expr * result = apply_trs(w.wr<>(et::lor, w.wrc(lisp_t), w.i64(11)));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(and nil (or (wr a 2) (wr b 1)))") {
  expr * result = apply_trs(w.wr<>(et::land,
    w.wrc(lisp_imm{nil_v}),
    w.wr<>(et::lor, w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2)))
  ));
  INFO(*result);
  assert_const(result, lisp_imm{nil_v}, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or True (and (wr a 1) (wr a 2)))") {
  expr * result = apply_trs(w.wr<>(et::lor,
    w.wrc(lisp_t),
    w.i64(11),
    w.wr<>(et::land, w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2)))
  ));
  INFO(*result);
  assert_const(result, lisp_t, "root");
}

TEST_CASE_FIXTURE(trs_fixture, "(or (or a b) (or c d))") {
  expr * result = apply_trs(w.wr<>(et::lor,
    w.wr<>(et::lor, w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2))),
    w.wr<>(et::lor, w.sym_wr(132391, w.i64(3)), w.sym_wr(197927, w.i64(4)))
  ));
  INFO(*result);
  lor * const root = static_cast<lor*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::lor, "Expected root to be lor.");
  input_span root_values = assert_inputs(root->values(), 4, "values");
  assert_sym_wr((root_values)[0].expr, 1319, 2, "values[0]", "values[0].value");
  assert_sym_wr((root_values)[1].expr, 66855, 4, "values[1]", "values[1].value");
  assert_sym_wr((root_values)[2].expr, 132391, 6, "values[2]", "values[2].value");
  assert_sym_wr((root_values)[3].expr, 197927, 8, "values[3]", "values[3].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(or a (or b c) d)") {
  expr * result = apply_trs(w.wr<>(et::lor,
    w.sym_wr(1319, w.i64(1)),
    w.wr<>(et::lor, w.sym_wr(66855, w.i64(2)), w.sym_wr(132391, w.i64(3))),
    w.sym_wr(197927, w.i64(4))
  ));
  INFO(*result);
  lor * const root = static_cast<lor*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::lor, "Expected root to be lor.");
  input_span root_values = assert_inputs(root->values(), 4, "values");
  assert_sym_wr((root_values)[0].expr, 1319, 2, "values[0]", "values[0].value");
  assert_sym_wr((root_values)[1].expr, 66855, 4, "values[1]", "values[1].value");
  assert_sym_wr((root_values)[2].expr, 132391, 6, "values[2]", "values[2].value");
  assert_sym_wr((root_values)[3].expr, 197927, 8, "values[3]", "values[3].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(or a Nil (or b False c) false d)") {
  expr * result = apply_trs(w.wr<>(et::lor,
    w.sym_wr(1319, w.i64(1)),
    w.wrc(lisp_imm{nil_v}),
    w.wr<>(et::lor, w.sym_wr(66855, w.i64(2)), w.wrc(lisp_f), w.sym_wr(132391, w.i64(3))),
    w.wrc(lisp_f),
    w.sym_wr(197927, w.i64(4))
  ));
  INFO(*result);
  lor * const root = static_cast<lor*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::lor, "Expected root to be lor.");
  input_span root_values = assert_inputs(root->values(), 4, "values");
  assert_sym_wr((root_values)[0].expr, 1319, 2, "values[0]", "values[0].value");
  assert_sym_wr((root_values)[1].expr, 66855, 4, "values[1]", "values[1].value");
  assert_sym_wr((root_values)[2].expr, 132391, 6, "values[2]", "values[2].value");
  assert_sym_wr((root_values)[3].expr, 197927, 8, "values[3]", "values[3].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(or (and a b) (or c d))") {
  expr * result = apply_trs(w.wr<>(et::lor,
    w.wr<>(et::land, w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2))),
    w.wr<>(et::lor, w.sym_wr(132391, w.i64(3)), w.sym_wr(197927, w.i64(4)))
  ));
  INFO(*result);
  lor * const root = static_cast<lor*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::lor, "Expected root to be lor.");
  input_span root_values = assert_inputs(root->values(), 3, "values");
  land * const root_values_0 = static_cast<land*>((root_values)[0].expr);
  J_REQUIRE_EQ_INFO(root_values_0->type, et::land, "Expected values[0] to be land.");
  input_span root_values_0_values = assert_inputs(root_values_0->values(), 2, "values[0].values");
  assert_sym_wr((root_values_0_values)[0].expr, 1319, 2, "values[0].values[0]", "values[0].values[0].value");
  assert_sym_wr((root_values_0_values)[1].expr, 66855, 4, "values[0].values[1]", "values[0].values[1].value");
  assert_sym_wr((root_values)[1].expr, 132391, 6, "values[1]", "values[1].value");
  assert_sym_wr((root_values)[2].expr, 197927, 8, "values[2]", "values[2].value");
}

TEST_CASE_FIXTURE(trs_fixture, "(and (or a b) (and c d))") {
  expr * result = apply_trs(w.wr<>(et::land,
    w.wr<>(et::lor, w.sym_wr(1319, w.i64(1)), w.sym_wr(66855, w.i64(2))),
    w.wr<>(et::land, w.sym_wr(132391, w.i64(3)), w.sym_wr(197927, w.i64(4)))
  ));
  INFO(*result);
  land * const root = static_cast<land*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::land, "Expected root to be land.");
  input_span root_values = assert_inputs(root->values(), 3, "values");
  lor * const root_values_0 = static_cast<lor*>((root_values)[0].expr);
  J_REQUIRE_EQ_INFO(root_values_0->type, et::lor, "Expected values[0] to be lor.");
  input_span root_values_0_values = assert_inputs(root_values_0->values(), 2, "values[0].values");
  assert_sym_wr((root_values_0_values)[0].expr, 1319, 2, "values[0].values[0]", "values[0].values[0].value");
  assert_sym_wr((root_values_0_values)[1].expr, 66855, 4, "values[0].values[1]", "values[0].values[1].value");
  assert_sym_wr((root_values)[1].expr, 132391, 6, "values[1]", "values[1].value");
  assert_sym_wr((root_values)[2].expr, 197927, 8, "values[2]", "values[2].value");
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("Lisp - AIR - TRS - Until");

TEST_CASE_FIXTURE(trs_fixture, "until((sym-wr a 1) empty-vec)") {
  expr * result = apply_trs(w.wr<>(et::do_until, w.progn(w.sym_wr(1319, w.i64(1)), w.wr<id>(et::sym_val_rd, 66343))));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  sym_val_rd * const root_forms_1 = static_cast<sym_val_rd*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::sym_val_rd, "Expected forms[1] to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_forms_1->var).raw == 66343, "Expected forms[1].var to equal %air:*empty-vec*");
}

TEST_CASE_FIXTURE(trs_fixture, "until((make-vec 10))") {
  expr * result = apply_trs(w.wr<>(et::do_until, w.wr<>(et::make_vec, w.i64(10))));
  INFO(*result);
  make_vec * const root = static_cast<make_vec*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::make_vec, "Expected root to be make_vec.");
  assert_const(root->size.expr, 20, "size");
}

TEST_CASE_FIXTURE(trs_fixture, "until((sym-wr a 1) (make-vec 10))") {
  expr * result = apply_trs(w.wr<>(et::do_until, w.progn(w.sym_wr(1319, w.i64(1)), w.wr<>(et::make_vec, w.i64(10)))));
  INFO(*result);
  progn * const root = static_cast<progn*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::progn, "Expected root to be progn.");
  input_span root_forms = assert_inputs(root->forms(), 2, "forms");
  assert_sym_wr((root_forms)[0].expr, 1319, 2, "forms[0]", "forms[0].value");
  make_vec * const root_forms_1 = static_cast<make_vec*>((root_forms)[1].expr);
  J_REQUIRE_EQ_INFO(root_forms_1->type, et::make_vec, "Expected forms[1] to be make_vec.");
  assert_const(root_forms_1->size.expr, 20, "forms[1].size");
}

TEST_CASE_FIXTURE(trs_fixture, "until(until(a))") {
  expr * result = apply_trs(w.wr<>(et::do_until, w.wr<>(et::do_until, w.wr<id>(et::sym_val_rd, 1319))));
  INFO(*result);
  do_until * const root = static_cast<do_until*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::do_until, "Expected root to be do_until.");
  sym_val_rd * const root_branch = static_cast<sym_val_rd*>(root->branch.expr);
  J_REQUIRE_EQ_INFO(root_branch->type, et::sym_val_rd, "Expected branch to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_branch->var).raw == 1319, "Expected branch.var to equal test:a");
}

TEST_CASE_FIXTURE(trs_fixture, "until(until(until(a)))") {
  expr * result = apply_trs(w.wr<>(et::do_until,
    w.wr<>(et::do_until, w.wr<>(et::do_until, w.wr<id>(et::sym_val_rd, 1319)))
  ));
  INFO(*result);
  do_until * const root = static_cast<do_until*>(result);
  J_REQUIRE_EQ_INFO(root->type, et::do_until, "Expected root to be do_until.");
  sym_val_rd * const root_branch = static_cast<sym_val_rd*>(root->branch.expr);
  J_REQUIRE_EQ_INFO(root_branch->type, et::sym_val_rd, "Expected branch to be sym_val_rd.");
  J_REQUIRE_UNARY_INFO((root_branch->var).raw == 1319, "Expected branch.var to equal test:a");
}
TEST_SUITE_END();
