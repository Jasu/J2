#include "lisp/reader/build_ast.hpp"
#include "logging/global.hpp"

#include "lisp/reader/state.hpp"
#include "lisp/env/static_ids.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "lisp/mem/heap.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/static_ids.hpp"

namespace j::lisp::reader {
  namespace {
    using namespace reader;

    struct J_TYPE_HIDDEN ast_inserter final {
      lisp_imm wrap(id name, lisp_imm child) {
        auto result = lisp_vec::allocate(heap, 2, (lisp::mem::object_hightag_flag)0, 1U);
        (*result)[0] = lisp_sym_id(name);
        (*result)[1] = child;
        return lisp_vec_ref(result);
      }

      lisp_imm operator()(const open_paren_token & t, token_iterator & it) {
        const u32_t sz = t.size;
        auto result = lisp_vec::allocate_with_debug_info(heap, t.source_location, sz, lisp::mem::object_hightag_flag::has_debug_info, 1U);
        ++it;
        for (u32_t i = 0; i < sz; ++i) {
          (*result)[i] = it->visit(*this, it);
        }
        J_ASSERT(it->type == token_type::close_paren, "Unexpected token type");
        ++it;

        return wrap_in_ellipses(lisp_vec_ref(result), t.has_pre_ellipsis, t.has_post_ellipsis);
      }

      lisp_imm operator()(const open_bracket_token &, const token_iterator &) {
        J_THROW("Unexpected bracket");
      }

      lisp_imm operator()(const close_paren_token &, const token_iterator &) {
        J_THROW("Unexpected close paren");
      }

      lisp_imm operator()(const close_bracket_token &, const token_iterator &) {
        J_THROW("Unexpected close bracket");
      }

      lisp_imm handle_bracket(lisp_imm imm, token_iterator & it) {
      again:
        if (it->type != token_type::open_bracket) {
          return imm;
        }
        ++it;
        auto result = lisp_vec::allocate(heap, 3, (lisp::mem::object_hightag_flag)0, 1U);
        (*result)[0] = lisp_sym_id(env::global_static_ids::id_at);
        (*result)[1] = imm;
        (*result)[2] = it->visit(*this, it);
        J_ASSERT(it->type == token_type::close_bracket, "Unexpected token type not ]");
        ++it;
        imm = lisp_vec_ref(result);
        goto again;
      }

      J_ALWAYS_INLINE lisp_imm operator()(const quote_token & t, token_iterator & it) {
        return wrap_in_ellipses(
          wrap(env::global_static_ids::id_quote, (++it)->visit(*this, it)),
          t.has_pre_ellipsis,
          false);
      }

      J_ALWAYS_INLINE lisp_imm operator()(const quasiquote_token & t, token_iterator & it)
      {
        return wrap_in_ellipses(
          wrap(env::global_static_ids::id_quasiquote, (++it)->visit(*this, it)),
          t.has_pre_ellipsis,
          false);
      }

      J_ALWAYS_INLINE lisp_imm operator()(const unquote_token &, token_iterator & it)
      { return wrap(env::global_static_ids::id_unquote, (++it)->visit(*this, it)); }

      J_ALWAYS_INLINE lisp_imm operator()(const symbol_token & t, token_iterator & it) {
        lisp_imm result = lisp_sym_id(t.id);
        result = wrap_in_ellipses(result, t.has_pre_ellipsis, t.has_post_ellipsis);
        ++it;
        result = handle_bracket(result, it);
        return result;
      }

      lisp_imm wrap_in_ellipses(lisp_imm imm, bool has_pre_ellipsis, bool has_post_ellipsis) {
        if (has_post_ellipsis) {
          imm = wrap(env::global_static_ids::id_post_ellipsis, imm);
        }
        return has_pre_ellipsis ? wrap(env::global_static_ids::id_pre_ellipsis, imm) : imm;
      }

      J_ALWAYS_INLINE lisp_imm operator()(const string_token & t, token_iterator & it) {
        return ++it, lisp_str_ref(t.value);
      }
      J_ALWAYS_INLINE lisp_imm operator()(const boolean_token & t, token_iterator & it) {
        return ++it, lisp_bool(t.value);
      }
      J_ALWAYS_INLINE lisp_imm operator()(const nil_token &, token_iterator & it) {
        return ++it, lisp_nil();
      }

      J_ALWAYS_INLINE lisp_imm operator()(const integer_token & t, token_iterator & it) {
        return ++it, lisp_i64(t.value);
      }

      J_ALWAYS_INLINE lisp_imm operator()(const floating_point_token & t, token_iterator & it) {
        return ++it, lisp_f32(t.value);
      }

      j::lisp::mem::heap & heap;
    };
  }

  lisp_imm build_form_ast(reader::state & s, token_iterator it) {
    ast_inserter ins{s.env->heap};
    return it->visit(ins, it);
  }

  lisp_vec * build_ast(reader::state & s) {
    const u32_t sz = s.num_root_elements;
    lisp_vec * result = lisp_vec::allocate(s.env->heap, sz, (lisp::mem::object_hightag_flag)0, 1U);
    ast_inserter ins{s.env->heap};
    auto it = s.begin();
    for (u32_t i = 0; i < sz; ++i) {
      (*result)[i] = it->visit(ins, it);
    }
    return result;
  }
}
