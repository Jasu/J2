#include "lisp/compilation/source_compilation.hpp"
#include "lisp/compilation/compilation_errors.hpp"
#include "lisp/compilation/compile_defun.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/static_ids.hpp"
#include "lisp/packages/parse_tlf.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/packages/parse_import.hpp"
#include "lisp/env/env_compiler.hpp"

namespace j::lisp::compilation {
  namespace {
    void handle_tlf_definition(source_compilation_context & src_compiler, packages::tlf_parse_result result) {
      if (result.type == packages::tlf_type::import) {
        packages::parse_import(&src_compiler.pkg, result.ast, &src_compiler.env_compiler);
        return;
      }

      value_info::value_info info;
      symbols::symbol_flags flags{symbols::symbol_flag::constant};

      switch (result.type) {
      case packages::tlf_type::fn:
        info = value_info::value_info(imm_fn_ref, value_info::function_value_flags);
        break;
      case packages::tlf_type::macro:
        info = value_info::value_info(imm_fn_ref, value_info::macro_value_flags);
        break;
      case packages::tlf_type::var:
        flags = {};
        [[fallthrough]];
      case packages::tlf_type::constant:
        info.types = any_imm_type;
        break;

      case packages::tlf_type::load_time_expr:
      case packages::tlf_type::pending:
        J_FAIL("Unsupported TLF definition type.");
      case packages::tlf_type::import:
        J_UNREACHABLE();
      }

      id id = result.ast.as_vec_ref()->at(1).as_sym_id();
      src_compiler.pkg.symbol_table.declare(id, result.ast, flags, info);
    }
  }

  tlf * source_compilation_context::push_tlf(lisp_imm form, tlf::tlf_status status) {
    last_tlf = &temp_pool.emplace<tlf>(tlf{
      .status = status,
      .form = form,
      .prev = last_tlf,
      .work_list = &work_list,
    });
    if (!first_tlf) {
      first_tlf = last_tlf;
    } else {
      last_tlf->prev->next = last_tlf;
    }
    return last_tlf;
  }

  void source_compilation_context::pend_tlf(struct id id, tlf * J_NOT_NULL tlf) {
    if (id.is_unresolved()) {
      if (struct id resolved = pkg.symbol_table.try_resolve(id)) {
        // Import in place, since the name resolved. Waiting for an other package.
        id = resolved;
        env_compiler.env->package_at(id.package_id())->symbol_table.add_pend(id, tlf);
      } else {
        // Pending for an import from some other package.
        // This is now the local package name.
        id = pkg.symbol_table.names.get_unresolved_ids(id).pkg_id;
        pkg.symbol_table.package_imports.add_pend(id, tlf);
      }
    } else {
      //Name is resolved?
      env_compiler.env->package_at(id.package_id())->symbol_table.add_pend(id, tlf);
    }
  }

  void source_compilation_context::compile_tlf(lisp_imm form) {
    packages::tlf_parse_result result = packages::parse_tlf(*this, form);

    if (result.type == packages::tlf_type::load_time_expr) {
      push_tlf(result.ast, tlf::tlf_status::parsed);
      ++num_expr_tlfs;
    } else if (result.type == packages::tlf_type::pending) {
      ++num_pending_tlfs;
      auto tlf = push_tlf(result.ast, tlf::tlf_status::pending);
      pend_tlf(result.pending_symbol, tlf);
    } else {
      handle_tlf_definition(*this, result);
      root.compile_pending();
    }
  }

  void source_compilation_context::compile_work_list() {
    while (work_list) {
      auto tlf = work_list;
      work_list = tlf->next_pending;
      tlf->next_pending = nullptr;

      J_ASSERT(tlf->status == tlf::tlf_status::pending);

      packages::tlf_parse_result result = packages::parse_tlf(*this, tlf->form);
      tlf->form = result.ast;

      if (result.type == packages::tlf_type::load_time_expr) {
        tlf->status = tlf::tlf_status::parsed;
        ++num_expr_tlfs;
        --num_pending_tlfs;
        continue;
      } else if (result.type == packages::tlf_type::pending) {
        pend_tlf(result.pending_symbol, tlf);
        continue;
      }

      --num_pending_tlfs;
      if (tlf->next) {
        tlf->next->prev = tlf->prev;
      } else {
        last_tlf = tlf->prev;
      }
      if (tlf->prev) {
        tlf->prev->next = tlf->next;
      } else {
        first_tlf = tlf->next;
      }

      handle_tlf_definition(*this, result);
    }
  }

  J_RETURNS_NONNULL symbols::symbol * source_compilation_context::to_final_ast() {
    J_ASSERT(!num_pending_tlfs);
    if (!num_expr_tlfs) {
      top_level_expr_symbol.value = lisp_nil_v;
      top_level_expr_symbol.value_info.types = imm_nil;
      top_level_expr_symbol.value_info.flags.clear(value_info::value_info_flag::function);
      return &top_level_expr_symbol;
    }

    lisp_vec * body = lisp_vec::allocate_with_debug_info(env_compiler.env->heap, source_location(), num_expr_tlfs + 3U, (lisp::mem::object_hightag_flag)0, 1U);
    body->at(0) = lisp_sym_id(env::global_static_ids::id_defun);
    body->at(1) = lisp_sym_id(top_level_expr_symbol.name);
    body->at(2) = air::g_empty_vec_ref;

    lisp_imm * to = &body->at(3);
    for (tlf * cur = first_tlf; cur; cur = cur->next) {
      J_ASSERT(cur->status == tlf::tlf_status::parsed);
      *to++ = cur->form;
    }
    top_level_expr_symbol.value = lisp_vec_ref(body);
    return &top_level_expr_symbol;
  }

  J_RETURNS_NONNULL symbols::symbol * source_compilation_context::to_defun() {
    auto sym = to_final_ast();
    if (sym->value_info.is_function()) {
      context ctx(pkg.get_pkg_context(), *sym);
      compile_defun(ctx);
    }
    return sym;
  }

  source_compilation_context::source_compilation_context(root_source_compilation_context & root,
                                                         pkg_context & package_context, source_compilation_context * next,
                                                         sources::source & src, symbols::symbol & top_level_expr_symbol)
    : pkg(package_context.pkg),
      src(src),
      env_compiler(root.env_compiler),
      temp_pool(env_compiler.temp_pool),
      next(next),
      top_level_expr_symbol(top_level_expr_symbol),
      root(root),
      package_context(package_context)
  {
    if (next) {
      next->prev = this;
    }
  }

  root_source_compilation_context::root_source_compilation_context(env::env_compiler & env_compiler, j::mem::bump_pool_checkpoint cp) noexcept
    : env_compiler(env_compiler),
      temp_pool(env_compiler.temp_pool),
      temp_checkpoint(cp)
  {
  }

  source_compilation_context & root_source_compilation_context::add_source_compiler(packages::pkg & pkg,
                                                                                    sources::source & src,
                                                                                    symbols::symbol * top_level_expr_symbol) {
    if (!top_level_expr_symbol) {
      top_level_expr_symbol = pkg.create_load_symbol();
    }

    first_context = &temp_pool.emplace<source_compilation_context>(*this, pkg.get_pkg_context(), first_context, src, *top_level_expr_symbol);
    return *first_context;
  }

  source_compiler_status source_compilation_context::compile(sources::source & src) {
    lisp_vec * ast = reader::read(env_compiler.env, &pkg, &src);
    J_ASSUME_NOT_NULL(ast);

    // Make a singly linked list of top-level forms at first.
    for (lisp_imm & imm : *ast) {
      compile_tlf(imm);
    }

    return num_pending_tlfs ? source_compiler_status::pending
      : source_compiler_status::ready;
  }

  void root_source_compilation_context::compile_pending() {
    for (;;) {
      bool did_compile = false;
      for (auto * src_context = first_context; src_context; src_context = src_context->next) {
        if (src_context->work_list) {
          did_compile = true;
          src_context->compile_work_list();
        }
      }
      if (!did_compile) {
        return;
      }
    }
  }

  source_compilation_context & get_source_compiler(env::env_compiler & env_compiler, packages::pkg & pkg, sources::source & src, symbols::symbol * top_level_expr_symbol) {
    auto & root = env_compiler.get_root_compilation_context();
    return root.add_source_compiler(pkg, src, top_level_expr_symbol);
  }

  symbols::symbol * compile_source(env::env_compiler & env_compiler, packages::pkg & pkg, sources::source & src, symbols::symbol * top_level_expr_symbol) {
    auto & src_compiler = get_source_compiler(env_compiler, pkg, src, top_level_expr_symbol);
    auto result = src_compiler.compile(src);
    J_ASSERT(result == source_compiler_status::ready);
    return src_compiler.to_final_ast();
  }
}
