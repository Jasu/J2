#include "gen_file.hpp"
#include "meta/module.hpp"
#include "files/ofile.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "lisp/env/context.hpp"

namespace j::meta {
  namespace {
#ifdef J_SRC_DIR
    constinit const char * const base_path{J_SRC_DIR};
#else
    constinit const char * const base_path{"."};
#endif
    void generate_from_template(strings::formatted_sink & sink, module * J_NOT_NULL mod, const generated_file_set & set, const generated_file & file) {
      lisp::env::env_ctx_guard g(lisp::env::env_context.enter(mod->test_env));
      auto root_ctx = mod->make_root_scope();
      expr_scope ctx{&root_ctx, set.scope_init};
      code_writer to(&sink);
      file.tpl.expand(to, ctx);
      to.begin_line();
    }
  }
  generated_file_set::generated_file_set(strings::const_string_view name, doc_comment && comment) noexcept
    : node(node_gen_file_set, name, static_cast<doc_comment &&>(comment))
  {
  }

  void generated_file_set::generate(module * J_NOT_NULL mod, strings::const_string_view suffix) const {
    for (auto & f : *this) {
      if (!suffix || f.suffix == suffix) {
        auto path = files::path(base_path) / f.path;
        auto ofile = j::mem::make_shared<files::ofile>(path);
        strings::formatted_sink sink{j::mem::static_pointer_cast<streams::sink>(ofile)};
        generate_from_template(sink, mod, *this, f);
      }
    }
  }

  void generated_file_set::generate(module * J_NOT_NULL mod, strings::formatted_sink & sink, strings::const_string_view suffix) const {
    for (auto & f : *this) {
      if (!suffix || f.suffix == suffix) {
        generate_from_template(sink, mod, *this, f);
      }
    }
  }

  void generated_file_set::generate(module * J_NOT_NULL mod, const files::path & to, strings::const_string_view suffix) const {
    auto ext = to.extension();
    files::path base = ext ? to.without_extension() : to;

    for (auto & f : *this) {
      if (!suffix || f.suffix == suffix) {
        auto path = base + "." + f.suffix;
        auto ofile = j::mem::make_shared<files::ofile>(path);
        strings::formatted_sink sink{j::mem::static_pointer_cast<streams::sink>(ofile)};
        generate_from_template(sink, mod, *this, f);
      }
    }
  }
}
