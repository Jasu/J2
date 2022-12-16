#include "meta_interface.hpp"
#include "meta/rules/graphviz_dump.hpp"
#include "strings/escape.hpp"
#include "meta/rules/id_resolver.hpp"
#include "meta/rules/rule.hpp"
#include "meta/expr.hpp"
#include "meta/rules/dump.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/pat_set.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"
#include "meta/module.hpp"
#include "meta/attr_context.hpp"
#include "meta/rules/pattern_tree.hpp"
#include "meta/rules/grammar.hpp"

namespace j::meta::inline rules {
  namespace s = strings;
  [[nodiscard]] attr_value trs_config::nt_data_type(const struct nt_data_type * dt) const noexcept {
    return dt ? nonterminal_data_types.as_tuple().at(dt->index) : attr_value();
  }
  [[nodiscard]] attr_value trs_config::nt_data_field(const struct nt_data_type * dt, u32_t index) const noexcept {
    J_ASSERT(dt);
    return nt_data_type(dt).struct_val.at("Fields").as_tuple().at(index);
  }
  namespace {
    struct J_TYPE_HIDDEN context final {
      parser_structs structs;
      module & mod;
      grammar & grammar;
      pat_sets states;
      i32_t num_terms = 0;
      i32_t num_terminals = 0;
      i32_t num_nonterminals = 0;
      u8_t exit_index = 0;
      expr_scope_root root_scope;
      noncopyable_vector<red_p> reductions;
      noncopyable_vector<attr_value> nonterminals;
    };

    void collect_reductions(context & ctx) {
      reds_hash_set_t h;
      for (auto & s : ctx.states) {
        if (s->unambiguous_action.is_reduce() && h.emplace(s->unambiguous_action.reduce).second) {
          ctx.reductions.emplace_back(s->unambiguous_action.reduce);
        } else if (s->unambiguous_non_exit_action.is_reduce() && h.emplace(s->unambiguous_non_exit_action.reduce).second) {
          ctx.reductions.emplace_back(s->unambiguous_non_exit_action.reduce);
          if (auto a = s->actions.maybe_at(exit)) {
            if (a->reduce && h.emplace(a->reduce).second) {
              ctx.reductions.emplace_back(a->reduce);
            }
          }
        } else {
          for (auto & p : s->actions) {
            if (p.second.reduce && h.emplace(p.second.reduce).second) {
              ctx.reductions.emplace_back(p.second.reduce);
            }
          }
        }
      }
    }
  }

  void register_parser_structs(module * J_NOT_NULL mod) {
    mod->define_node(make_enum("ReductionType", {"Reduce", "ReduceCopy", "Rewrite", "Erase", "Truncate", "Stmt", "ChangeType", "Pop"}));
    mod->define_node(make_enum("RewriteAction", {"None", "Replace"}));
    mod->define_node(::new struct_def("NonTerminal", (const char*[]){
      "Index", "Name", "DataType", "Fields",
    }));
    mod->define_node(::new struct_def("NtDataType", (const char*[]){"Name", "Fields", "Size"}));
    mod->define_node(::new struct_def("NtField", (const char*[]){"Name", "Type", "Initializer"}));
    mod->define_node(::new struct_def("RewriteStep", (const char*[]){"RewriteAction", "Offset", "Value"}));
    mod->define_node(::new struct_def("Goto", (const char*[]){"OnNonTerminal", "DestinationState"}));
    mod->define_node(::new struct_def("State", (const char*[]){"Actions", "Gotos",
          "UnambiguousAction", "UnambiguousNonExitAction"}));
    mod->define_node(::new struct_def("Tables", (const char*[]){
          "ErrorAction",
          "States", "NonTerminals", "Reductions",
          "NonTerminalDataTypes",
          "NumTerminals", "ExitToken",
        }));
    mod->define_node(::new struct_def("ReduceInfos", (const char*[]){"ReductionGroups", "Precomputes", "HasStatements"}));
    mod->define_node(::new struct_def("ReductionGroup", (const char*[]){"Predicate", "Reductions", "Description"}));
    mod->define_node(::new struct_def("Precompute", (const char*[]){"Type", "Name", "Value"}));
    mod->define_node(::new struct_def("Reduction", (const char*[]){
          "Type", "ChangesLookahead",
          "StackDepth", "TreeDepth", "Statements",
          "Nt", "NtType", "Args",
          "Debug",
        }));
  }

  void trs_config::define(module * J_NOT_NULL mod) {
    te_nt_data_source_enum = (enum_def*)mod->define_node(
      make_enum("TeDataSource", {
                "None", "Opaque", "Const", "ConvertedConst",
                "StackTerm", "StackNt", "StackNtField",
                "ConstructTerm"}));

    te_nt_struct = ::new struct_def("TeNtData", (const char*[]){
      "DataMode", "Type",
      // Copy (also for state mode)
      "StackOffset",
      // Construct
      "Args",

      "StateMode", "Nt",
    });

    te_value_struct = ::new struct_def("TeValue", (const char*[]){
      "Source", "Value", "Type",
      "StackOffset", // Stack term / Stack NT / Stack NT field
      "FieldOffset", // Stack NT field
      "InputOffset", // Input term
      "Size", // Stack NT blob
      "Term", // Ctor
      "Args", // Ctor
      "Original", // Converted const
    });

    nt_field_struct = (struct_def*)mod->node_at("NtField", node_struct_def);
    nt_type_struct = (struct_def*)mod->node_at("NtDataType", node_struct_def);
    mod->define_node(te_value_struct);
  }

  parser_structs get_parser_structs(module * J_NOT_NULL mod) {
    return {
      .reduction_type = (enum_def*)mod->node_at("ReductionType", node_enum),
      .rewrite_action = (enum_def*)mod->node_at("RewriteAction", node_enum),
      .tables = (struct_def*)mod->node_at("Tables", node_struct_def),
      .goto_table = (struct_def*)mod->node_at("Goto", node_struct_def),
      .state = (struct_def*)mod->node_at("State", node_struct_def),
      .non_terminal = (struct_def*)mod->node_at("NonTerminal", node_struct_def),
      .rewrite_step = (struct_def*)mod->node_at("RewriteStep", node_struct_def),
      .reduce_infos = (struct_def*)mod->node_at("ReduceInfos", node_struct_def),
      .reduce_group = (struct_def*)mod->node_at("ReductionGroup", node_struct_def),
      .reduction = (struct_def*)mod->node_at("Reduction", node_struct_def),
      .precompute = (struct_def*)mod->node_at("Precompute", node_struct_def),
      .nt_field = (struct_def*)mod->node_at("NtField", node_struct_def),
      .nt_data_type = (struct_def*)mod->node_at("NtDataType", node_struct_def),
    };
  }

  namespace {
    attr_value make_reduction_args(context & ctx, const reduction & r, const reduction_args & args, precomputes & precs) {
      i32_t sz = args.size();
      attr_value rewrite_info[sz];
      i32_t i = 0U;
      if (sz) {
        te_type types[sz];
        switch (r.type) {
        case reduction_type::truncate:
        case reduction_type::erase:
        case reduction_type::pop:
          J_UNREACHABLE();

        case reduction_type::change_type:
          J_ASSERT(sz == 2);
          types[0] = te_type(ctx.mod.trs_term_type);
          types[1] = te_type(ctx.mod.trs_term_type_type);
          break;
        case reduction_type::stmt:
          J_ASSERT(sz == 1);
          types[0] = te_type::any;
          break;
        case reduction_type::rewrite:
          for (; i < sz; ++i) {
            types[i] = te_type(term_expr_type_kind::term_or_terms);
          }
          break;
        case reduction_type::reduce: {
          if (nt_data_type * dt =  r.data_type ? r.data_type : r.nt->data_type.get()) {
            for (auto & f : dt->fields) {
              if (i == sz) {
                break;
              }
              types[i++] = f.type;
            }
          } else {
            for (; i < sz; ++i) {
              types[i] = te_type(term_expr_type_kind::term_or_terms);
            }
          }
          break;
        }
        case reduction_type::reduce_copy:
          J_ASSERT(sz == 1);
          types[0] = te_type(term_expr_type_kind::blob, r.size ? r.size : r.data_type ? r.data_type->size : (r.nt && r.nt->data_type ? r.nt->data_type->size : 0));
          break;
        }

        i = 0U;
        for (const auto & arg : args) {
          rewrite_info[i++] = attr_value(ctx.structs.rewrite_step, (pair<const char*, attr_value>[]){
              {"RewriteAction", attr_value(ctx.structs.rewrite_action, (u32_t)arg.rewrite_action)},
              {"Offset", arg.rewrite_action != rewrite_action::none ? attr_value(arg.rewrite_offset) : attr_value()},
              {"Value", arg.expr->compile(ctx.root_scope, precs, types[i])},
            });
        }
      }
      return attr_value(span(rewrite_info, i));
    }

    attr_value make_precomputes(context & ctx, precomputes & precs) {
      attr_value_array result(uninitialized, precs.all.size());
      for (auto & prec : precs.all) {
        if (prec->value) {
          result.initialize_element(ctx.structs.precompute, (pair<const char*, attr_value>[]){
              {"Name", attr_value(prec->name)},
              {"Type", attr_value(prec->type)},
              {"Value", attr_value(prec->value)},
            });
        }
      }
      return attr_value((attr_value_array &&)result);
    }

    attr_value make_reduction_group(context & ctx, const reduction_group & group, precomputes precs, bool & has_statements) {
      attr_value reds[group.size()];
      i32_t i = 0;
      for (auto & r : group) {
        nt_p nt = r.nt;
        nt_data_type * dt = nt ? nt->data_type.get() : r.data_type;
        bool changes_lookahead = false;
        if (r.type == reduction_type::rewrite) {
          for (auto & arg : r.args) {
            if (arg.rewrite_action == rewrite_action::replace) {
              changes_lookahead |= arg.rewrite_offset == 1;
            }
          }
        }
        i32_t stmt_idx = 0;
        make_reduction_args(ctx, r, r.args, precs);
        attr_value_array statement_vals(uninitialized, r.statements.size() + precs.all.size());
        if (i == 0) {
          for (auto prec : precs.all) {
            statement_vals.initialize_element(s::format("{} {} = {}", prec->type, prec->name, prec->value));
          }
        }
        for (auto & stmt : r.statements) {
          if (stmt.consuming_te) {
            auto val = stmt.consuming_te->compile_raw(ctx.root_scope, precs);
            statement_vals.initialize_element(s::format("{} {}stmt_{} = {}", val.type.value_type->type_name, stmt.by_ref ? "& " : "", stmt_idx, val.as_str(0)));
            precs.add_precompute(stmt.consuming_te, s::format("stmt_{}", stmt_idx), s::string(val.type.value_type->type_name) + (stmt.by_ref ? "& " : ""), val.as_str(0));
            ++stmt_idx;
          } else {
            statement_vals.initialize_element(stmt.release_expr->compile_str(ctx.root_scope, precs));
          }
        }

        has_statements |= statement_vals.size();
        attr_value args{make_reduction_args(ctx, r, r.args, precs)};
        reds[i++] = attr_value(ctx.structs.reduction, (pair<const char*, attr_value>[]){
          {"Type", attr_value(ctx.structs.reduction_type, (u32_t)r.type)},
          {"StackDepth", attr_value(r.length)},
          {"TreeDepth", attr_value(r.depth_difference)},
          {"Statements", attr_value((attr_value_array&&)statement_vals)},
          {"Nt", nt ? ctx.nonterminals.at(nt->index) : attr_value(false)},
          {"NtType", (dt && dt->size) ? ctx.mod.trs_config.nt_data_type(dt) : attr_value()},
          {"Args", (attr_value&&)args},
          {"ChangesLookahead", attr_value(changes_lookahead)},
        });
      }
      return attr_value(ctx.structs.reduce_group, (pair<const char*, attr_value>[]){
          {"Reductions", attr_value(span(reds, i))},
          {"Predicate", group.predicate ? group.predicate->compile(ctx.root_scope) : attr_value{}},
          {"Description", attr_value(s::escape_double_quoted(dump_str(&ctx.mod, group)))},
        });
    }

    attr_value make_reduction_info(context & ctx, const red_p & reduce) {
      attr_value groups[reduce->size()];
      u32_t i = 0U;
      arg_counts counts; // = reduce->get_arg_counts();
      // counts.filter_min(3);
      for (auto & g : *reduce) {
        hash_set<i32_t> written;
        for (auto & r : g) {
          for (const auto & arg : r.args) {
            auto reads = arg.expr->collect_args();
            for (const auto & read : reads) {
              if ((read.first.type == arg_type::capture_term || read.first.type == arg_type::select_term_ahead)
                  && written.contains(-read.first.stack_offset))
              {
                counts.add(read.first);
              }
            }
            if (arg.rewrite_action != rewrite_action::none) {
              written.emplace(arg.rewrite_offset);
            }
          }
        }
      }

      precomputes precs;//  = counts.as_precomputes(ctx.mod);
      i = 0;
      bool has_statements= false;
      for (auto & g : *reduce) {
        groups[i++] = make_reduction_group(ctx, g, counts.as_precomputes(ctx.mod), has_statements);
      }
      return attr_value(ctx.structs.reduce_infos, (pair<const char*, attr_value>[]){
          {"ReductionGroups", attr_value(span(groups, i))},
          {"Precomputes", make_precomputes(ctx, precs)},
          {"HasStatements", attr_value(has_statements)},
        });
    }

    [[nodiscard]] attr_value reduction_index(const context & ctx, const red_p & red, bool is_shift_reduce) noexcept {
      for (i64_t j = 0; j < ctx.reductions.size(); ++j) {
        if (*ctx.reductions[j] == *red) {
          return attr_value(~(j + (is_shift_reduce ? 1 << 16 : 0)));
        }
      }
      J_FAIL("No reduction", dump_str(&ctx.mod, red));
    }

    attr_value action_to_index(context & ctx, const action & act, bool default_zero) {
      switch (act.type) {
      case action_type::none:
        return default_zero ? attr_value(0x7FFFFFFF) : attr_value();
      case action_type::shift:
        return attr_value(act.to->index);
      case action_type::reduce: case action_type::shift_reduce:
        return reduction_index(ctx, act.reduce, act.type == action_type::shift_reduce);
      }
    }


    [[nodiscard]] attr_value make_action_table_state(context & ctx, const pat_set_pp & state) {
      u32_t num_actions = ctx.num_terminals;
      attr_value actions[num_actions];
      for (u8_t i = 0U; i < num_actions; ++i) {
        terminal t = i == ctx.exit_index ? exit : terminal{i};
        if (state->unambiguous_action) {
          actions[i] = action_to_index(ctx, state->unambiguous_action, true);
        } else if (t != exit && state->unambiguous_non_exit_action) {
          actions[i] = action_to_index(ctx, state->unambiguous_non_exit_action, true);
        } else {
          const action * act = state->actions.maybe_at(t);
          actions[i] = act ? action_to_index(ctx, *act, true) : attr_value(0x7FFFFFFF);
        }
      }
      return attr_value(span(actions, num_actions));
    }

    attr_value make_parser_state(context & ctx, const pat_set_pp & state) {
      const u32_t num_gotos = state->gotos.size();
      attr_value gotos[num_gotos];
      u32_t i = 0U;
      for (auto & p : state->gotos) {
        gotos[i++] = attr_value(ctx.structs.goto_table, (pair<const char*, attr_value>[]){
          {"OnNonTerminal", attr_value(p.first->name)},
          {"DestinationState", attr_value(p.second->index)}});
      }
      return attr_value(ctx.structs.state, (pair<const char*, attr_value>[]){
          {"UnambiguousAction", action_to_index(ctx, state->unambiguous_action, false)},
          {"UnambiguousNonExitAction", action_to_index(ctx, state->unambiguous_action ? state->unambiguous_action : state->unambiguous_non_exit_action, false)},
          {"Actions", make_action_table_state(ctx, state)},
          {"Gotos", attr_value(span(gotos, num_gotos))},
      });
    }

    [[nodiscard]] attr_value make_nt_fields(context & ctx, const vector<nt_data_field> & fields, bool is_global) noexcept {
      const i32_t num_fields = fields.size();
      attr_value result[num_fields];
      int i = 0;
      for (auto & f : fields) {
        attr_value initializer;
        if (!is_global && f.initializer) {
          f.initializer->resolve(ctx.root_scope, ctx.mod.normal_resolver);
          initializer = attr_value(f.initializer->compile_str(ctx.root_scope, f.type));
        }
        result[i++] = attr_value(ctx.structs.nt_field, (pair<const char*, attr_value>[]){
            {"Name", attr_value(f.name)},
            {"Type", attr_value((node*)f.type)},
            {"Initializer", static_cast<attr_value &&>(initializer)}
        });
      }
      return attr_value(span(result, num_fields));
    }


    void make_nonterminal_data_types(context & ctx) {
      auto & types = ctx.mod.nt_types;
      const u32_t num_data_types = types.size();
      attr_value data_types[num_data_types];
      for (u32_t i = 0U; i != num_data_types; ++i) {
        auto & type = types[i];
        data_types[i] = attr_value(ctx.structs.nt_data_type, (pair<const char*, attr_value>[]){
            {"Name", attr_value(type->name)},
            {"Fields", make_nt_fields(ctx, type->fields, true)},
            {"Size", attr_value(type->size)},
          });
      }
      ctx.mod.trs_config.nonterminal_data_types = attr_value(span(data_types, num_data_types));
    }

    [[nodiscard]] attr_value make_parser_tables(context & ctx) {
      const u32_t num_states = ctx.states.size();
      const u32_t num_reductions = ctx.reductions.size();
      attr_value states[num_states];
      attr_value reductions[num_reductions];
      for (u32_t i = 0U; i != num_states; ++i) {
        states[i] = make_parser_state(ctx, ctx.states[i]);
      }
      for (i32_t i = 0U; i != ctx.num_nonterminals; ++i) {
        nt_p nt = ctx.mod.nonterminals().at(i);
        J_ASSERT(i == nt->index);
        attr_value fields;
        if (nt->data_type) {
          fields = make_nt_fields(ctx, nt->data_type->fields, false);
        }
        ctx.nonterminals.emplace_back(ctx.structs.non_terminal, (pair<const char*, attr_value>[]){
            {"Index", attr_value(nt->index)},
            {"Name", attr_value(nt->name)},
            {"DataType", ctx.mod.trs_config.nt_data_type(nt->data_type.get())},
            {"Fields", static_cast<attr_value &&>(fields)},
          });
      }
      for (u32_t i = 0U; i != num_reductions; ++i) {
        reductions[i] = make_reduction_info(ctx, ctx.reductions[i]);
      }
      return attr_value(ctx.structs.tables, (pair<const char*, attr_value>[]){
          {"States", attr_value(span(states, num_states))},
          {"ErrorAction", attr_value(0x7FFFFFFF)},
          {"NonTerminals", attr_value(span(ctx.nonterminals.begin(), ctx.num_nonterminals))},
          {"NonTerminalDataTypes", ctx.mod.trs_config.nonterminal_data_types},
          {"Reductions", attr_value(span(reductions, num_reductions))},
          {"NumTerminals", attr_value(ctx.num_terminals)},
          {"ExitToken", attr_value(ctx.exit_index)},
      });
    }
  }

  attr_value get_parser_tables(module * J_NOT_NULL mod) {
    context ctx{
      .structs = get_parser_structs(mod),
      .mod = *mod,
      .grammar = *mod->grammar,
      .states{*mod->grammar},
      .num_terms = (u8_t)mod->term_ptrs.size(),
      .num_terminals = mod->grammar->terminals.size + 1,
      .num_nonterminals = (u8_t)mod->nonterminal_ptrs.size(),
      .exit_index = mod->grammar->terminals.size,
      .root_scope = mod->make_root_scope(),
    };
    collect_reductions(ctx);
    // Make data types first, they are required for other fields.
    make_nonterminal_data_types(ctx);
    return make_parser_tables(ctx);
  }
}
