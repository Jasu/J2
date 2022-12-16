#include "cli/cli.hpp"
#include "meta/rules/graphviz_dump.hpp"
#include "meta/module.hpp"
#include "meta/rules/dump.hpp"
#include "meta/rules/grammar.hpp"
#include "meta/expr.hpp"
#include "meta/gen_file.hpp"
#include "meta/dump.hpp"
#include "meta/term.hpp"
#include "strings/styling/default_styles.hpp"
#include "exceptions/assert.hpp"
#include "strings/string_algo.hpp"
#include "meta/module.hpp"
#include "meta/rules/rule.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "files/paths/path.hpp"

namespace j::commands {
  namespace {
    namespace m = j::meta;
    namespace s = j::strings;

#ifdef J_RES_DIR
    const files::path res_dir(J_RES_DIR);
#else
    const files::path res_dir(".");
#endif
    struct J_TYPE_HIDDEN dump_ctx final {
      s::formatted_sink & stdout;
      m::module & mod;
      s::const_string_view lalr_debug;
      s::const_string_view lalr_debug_states;
      m::code_writer writer;
      m::dump_context ctx;

      J_A(AI,ND) dump_ctx(s::formatted_sink & stdout, m::module & mod, s::const_string_view lalr_debug, s::const_string_view lalr_debug_states) noexcept
        : stdout(stdout),
        mod(mod),
        lalr_debug(lalr_debug),
        lalr_debug_states(lalr_debug_states),
        writer{&stdout},
        ctx(writer, &mod)
      { }
    };

    struct J_TYPE_HIDDEN dumper final {
      strings::const_string_view name;
      void (*dump)(dump_ctx) = nullptr;
    };

    void dump_terms(dump_ctx ctx) {
      bool is_first = true;
      for (const m::term * t : ctx.mod.terms()) {
        if (!is_first) {
          ctx.stdout.write_unformatted("\n");
        }
        is_first = false;
        m::dump(ctx.ctx, *t);
      }
    }

    void dump_graphviz(dump_ctx ctx) {
      auto g = ctx.mod.grammar;
      m::graphviz_dump("foo.dot", *g);
    }

    void dump_rules(dump_ctx ctx) {
      for (auto & r : ctx.mod.nonterminals()) {
        m::dump(ctx.ctx, *r);
      }
      ctx.stdout.write("\n");
      // for (auto & r : ctx.mod.rules()) {
      //   m::dump(ctx.ctx, *r);
      // }
      // ctx.stdout.write("\n");
    }

    void dump_vars(dump_ctx ctx) {
      auto root = ctx.mod.make_root_scope();
      for (auto & r : ctx.mod.root_scope_init.vars) {
        ctx.writer.write_formatted("{#bright_yellow}Def{/} {#bright_magenta,bold}{}{/} = ", r.first);
        ctx.writer.descend();
        m::dump(ctx.ctx, *r.second);
        ctx.writer.ascend();

        if (r.second->type != m::expr_const)  {
          ctx.writer.write("# Evaluated: ", s::styles::light_gray);
          ctx.writer.descend();
          m::dump(ctx.ctx, root.get(r.first));
          ctx.writer.ascend();
          ctx.writer.queue_break();
        }
        ctx.writer.queue_empty_line();
      }
    }

    void write_line(dump_ctx & ctx, const char * J_NOT_NULL name, const m::attr_value & v) {
      if (v) {
        auto i = ctx.ctx.to.indent;
        ctx.ctx.to.begin_line();
        ctx.ctx.to.write(name, s::styles::bright_magenta);
        ctx.ctx.to.indent += j::strlen(name);
        m::dump(ctx.ctx, v);
        ctx.ctx.to.indent = i;
      }
    }

    void write_line(dump_ctx & ctx, const char * J_NOT_NULL name, strings::const_string_view v) {
      if (v) {
        ctx.ctx.to.write_line_formatted("{#bright_cyan}{}{/}{#light_gray,bold}\"{}\"{/}", name, v);
      }
    }

    void write_line(dump_ctx & ctx, const char * J_NOT_NULL name, i64_t n) {
      ctx.ctx.to.write_line_formatted("{#bright_yellow}{}{/}{}", name, n);
    }

    void dump_types(dump_ctx ctx) {
      m::comment_writer cw{ctx.writer, m::hash_comment};
      bool is_first = true;
      for (const m::val_type * t : ctx.mod.val_types()) {
        if (!is_first) {
          ctx.stdout.write_unformatted("\n");
        }
        is_first = false;

        cw.write_line_formatted("{#light_gray}Type {#white}#{#bold}{}{/} `{#bold}{}{/}`{/}:{/}",
                                t->index, t->name);
        cw.write_doc_comment(t->comment);
        cw.reset();

        ctx.stdout.write("{#bright_cyan,bold}{}Type{/} {#bright_yellow,bold}{}{/} \\{\n",
                     t->allowed_regions == m::region_data_or_const
                     ? "Const" : (t->size ? "In" : "Ins"),
                     t->name);

        ctx.writer.descend();

        write_line(ctx, "Type            ", t->type_name);
        if (t->ctor_type_name != t->type_name) {
          write_line(ctx, "CtorType        ", t->ctor_type_name);
        }
        write_line(ctx, "CtorStmt        ", t->ctor_stmt);
        write_line(ctx, "GetterType      ", t->getter_type_name);
        write_line(ctx, "GetterExpr      ", t->getter_expr);
        if (t->const_getter_expr != t->getter_expr) {
          write_line(ctx, "ConstGetterExpr ", t->getter_expr);
        }
        if (t->const_getter_type_name != t->ctor_type_name) {
          write_line(ctx, "ConstGetterType ", t->const_getter_type_name);
        }
        if (t->size) {
          write_line(ctx, "Size            ", t->size);
        } else {
          write_line(ctx, "SizeExpr        ", t->size_expr);
          if (t->ctor_size_expr != t->size_expr) {
            write_line(ctx, "CtorSizeExpr    ", t->ctor_size_expr);
          }
        }
        if (t->size != t->align) {
          write_line(ctx, "Align           ", t->align);
        }
        ctx.writer.ascend();
        ctx.writer.write_line("}");
      }
    }

    const dumper dumpers[]{
      { "terms",    &dump_terms },
      { "vars",     &dump_vars },
      { "graphviz", &dump_graphviz },
      { "rules",    &dump_rules },
      { "types",    &dump_types },
    };

    void print_available(s::formatted_sink & stdout, m::module * J_NOT_NULL mod) noexcept {
      stdout.write("\n{#red_bg,bright_yellow,bold} Available files for {#bright_cyan}--generate{/} and {#bright_cyan}--dump{/}:               {/}\n");
      stdout.write("\n{#magenta_bg,bright_yellow,bold} Special:                                                         {/}\n");
      stdout.write("  {#bold,bright_yellow}all{/} maps to all files.\n");
      for (auto & d : dumpers) {
        stdout.write("  {#bold,bright_yellow}{}{/} is only available for {#bright_cyan,bold}--dump{/}\n", d.name);
      }
      stdout.write("\n{#green_bg,white,bold} Defined in .meta:                                                {/}\n");
      for (auto fs : mod->gen_file_sets()) {
        stdout.write("  {#bold}{}{/}:\n", fs->name);
        for (const m::generated_file & f : *fs) {
          stdout.write("    {#bright_yellow}{:>37}{/} ({}.{})\n", f.path, fs->name, f.suffix);
        }
      }
    }

    bool handle_single(s::formatted_sink & stdout, m::module * J_NOT_NULL mod,
                         strings::const_string_view name, const files::path & output, bool generate) {
      for (auto f : mod->gen_file_sets()) {
        if (f->name == name) {
          if (generate) {
            if (output) {
              f->generate(mod, output, "");
            } else {
              f->generate(mod, "");
            }
          } else {
            f->generate(mod, stdout, "");
          }
          return true;
        }
      }
      stdout.write("{#bold}{#bright_red}Generated file{/} {#bright_yellow,red_bg} {} {/} {#bright_red}was not found.{/}{/}", name);
      print_available(stdout, mod);
      return false;
    }

    void meta(mem::shared_ptr<s::formatted_sink> stdout,
              mem::shared_ptr<s::formatted_sink> stderr,
              files::path input,
              files::path output,
              strings::const_string_view dump,
              strings::const_string_view generate,
              strings::const_string_view debug_lalr,
              strings::const_string_view debug_lalr_states,
              strings::const_string_view debug_node,
              bool debug_print) {
      J_ASSERT_NOT_NULL(stdout, input);
      stderr->write("Parsing expressions from {#bright_green,bold}{}{/}...\n", input);
      stderr->flush();
      m::module mod(res_dir);
      mod.debug_node = debug_node;
      mod.parse_path(input);
      mod.set_global_bool("DebugPrint", debug_print);

      // if (op) {
      //   J_REQUIRE(dump, "--op is only allowed for --dump.");
      //   for (auto it = mod.terms.begin(); it != mod.terms.end();) {
      //     if ((*it)->name != op) {
      //       it = mod.terms.erase(it);
      //     } else {
      //       ++it;
      //     }
      //   }
      // }

      s::const_string_view name = dump ? dump : generate;
      J_REQUIRE(name, "Name must be given.");

      if (dump) {
        for (auto & d : dumpers) {
          if (d.name == name) {
            d.dump(dump_ctx{*stdout, mod, debug_lalr, debug_lalr_states});
            return;
          }
        }
      }

      bool result = false;

      if (name == "all") {
        J_REQUIRE(!output, "When processing all files, --output cannot be specified.");
        J_TODO();
      } else {
        result = handle_single(*stdout, &mod, name, output, (bool)generate);
      }

      if (!result) {
        stdout->write("{#bright_red,bold}Failed to generate{/} {#bright_yellow,red_bg,bold} {} {/}\n", name);
      }
    }

    namespace c = cli;
    namespace sv = services;
    J_A(NODESTROY) const sv::service_instance<cli::command> cmd(
      "commands.meta",
      "Metacompiler",
      c::command_name = "meta",
      c::callback     = sv::call(&meta,
                                 sv::service("stdout"),
                                 sv::service("stderr"),
                                 c::argument("input"),
                                 c::argument("output", c::default_value = ""),
                                 c::option("dump", c::default_value = ""),
                                 c::option("generate", c::default_value = ""),
                                 c::option("debug-lalr", c::default_value = ""),
                                 c::option("debug-lalr-states", c::default_value = ""),
                                 c::option("debug-node", c::default_value = ""),
                                 c::flag("debug-print")
        ));

                                 // c::option("debug-lalr", c::default_value = ""),
                                 // c::option("debug-lalr-states", c::default_value = ""),
                                 // c::option("node", c::default_value = ""),
                                 // c::flag("debug-print")));
  }
}
