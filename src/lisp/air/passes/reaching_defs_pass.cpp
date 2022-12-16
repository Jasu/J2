#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/accessors.hpp"
#include "containers/trivial_array.hpp"
#include "logging/global.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "bits/bitset.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    using namespace exprs;

    struct J_A(HIDDEN) branch_data final {
      /// Uses of variables that depend on later loop outputs.
      ///
      /// Bitmask index is a synthetic index of the use.
      bits::bitset exposed_uses;
      /// Defs that are currently visible, of all variables.
      ///
      /// Bitmask index is a synthetic index of the def.
      bits::bitset active_defs;
      /// Vars that are conclusively defined by this branch.
      ///
      /// I.e. any variable that is defined and does not depend on loop output is here.
      ///
      /// Bitmask index is a synthetic index of the air_var.
      bits::bitset defined_vars;

      void add_alternative_branch(const branch_data & rhs) noexcept {
        exposed_uses |= rhs.exposed_uses;
        active_defs |= rhs.active_defs;
        defined_vars &= rhs.defined_vars;
      }

      void add_optional_successor(const branch_data & rhs) noexcept {
        exposed_uses |= rhs.exposed_uses;
        active_defs |= rhs.active_defs;
      }
    };

    struct J_A(HIDDEN) var_data final {
      exprs::expr * expr = nullptr;
      u32_t air_var_index = 0U;
      u32_t pass_index = 0U;
      bits::bitset accesses;

      template<typename BitSetInit>
      J_A(AI,ND) explicit var_data(exprs::expr * J_NOT_NULL e, u32_t air_var_index, u32_t pass_index, BitSetInit && accesses)
        : expr(e),
          air_var_index(air_var_index),
          pass_index(pass_index),
          accesses(static_cast<BitSetInit &&>(accesses))
      { }
    };

    struct J_A(HIDDEN) debug_print_guard final {
      expr_type scope_type = expr_type::none;
      u32_t * indent = nullptr;
      J_A(AI,ND) constexpr debug_print_guard() noexcept = default;
      debug_print_guard(expr_type scope_type, u32_t * J_NOT_NULL indent)
        : scope_type(scope_type),
          indent(indent)
      {
        J_DEBUG("{indent}{#bright_green_bg,white} Entering {#green_bg,bold} {} {/} {/}", *indent, scope_type);
        *indent += 2U;
      }

      void print(const char * J_NOT_NULL message) noexcept {
        if (indent) {
          J_DEBUG("{indent}{#bright_magenta_bg,white,bold} {} {#magenta_bg} {} {/} {/}", *indent, message, scope_type);
        }
      }

      ~debug_print_guard() {
        if (indent) {
          *indent -= 2U;
          J_DEBUG("{indent}{#light_gray_bg,white} Exiting {#gray_bg,bold} {} {/} {/}", *indent, scope_type);
        }
      }
    };

    struct J_A(HIDDEN) reaching_defs_visitor final {
      j::mem::bump_pool & pool;

      u32_t num_uses, num_defs;
      /// Contains the expr* and the defs used by each lex_rd.
      ///
      /// Indexed by a generated index.
      trivial_array<var_data> uses;
      /// Contains lex_wr expr*s and the uses using them.
      ///
      /// Indexed by a generated index.
      trivial_array<var_data> defs;

      trivial_array<bits::bitset> var_defs;

      branch_data root_data;

      u32_t indent = 0U;
      bool enable_debug;

      [[nodiscard]] debug_print_guard debug_enter(expr_type type) {
        return enable_debug ? debug_print_guard(type, &indent) : debug_print_guard();
      }

      template<typename... Args>
      J_A(ND, HIDDEN) inline void debug(const char * J_NOT_NULL fmt, Args && ... args) noexcept {
        if (enable_debug) {
          J_DEBUG(strings::string("{indent}") + fmt, indent, static_cast<Args &&>(args)...);
        }
      }

      void on_use(expr * J_NOT_NULL e, branch_data & data) noexcept {
        J_ASSERT(is_lex_read(e->type));
        const air_var & v = get_lex_var(e);
        auto & use = uses.initialize_element(e, v.index, uses.size(), var_defs.at(v.index));
        use.accesses &= data.active_defs;
        bool is_defined = data.defined_vars[v.index];
        debug("{#yellow_bg,black,bold} {} {/} of var {} by {}. (Air var index #{}, Use index #{})",
              is_defined ? "Use" : "Upward-exposed use",
              v.name, *e, (u32_t)v.index, use.pass_index);
        if (is_defined) {
          write_use_defs(use);
        } else {
          data.exposed_uses.set(use.pass_index);
        }
      }

      void on_def(expr * J_NOT_NULL e, branch_data & data) noexcept {
        const air_var & v = get_lex_var(e);
        J_ASSERT(is_lex_write(e->type));
        auto & def = defs.initialize_element(e, v.index, defs.size(), num_uses);

        debug("{#green_bg,bold,white} Def {/} of var {} by {}. (Air var index #{}, Def index #{})", v.name, *e, (u32_t)v.index,  def.pass_index);

        // Remove all other defs of the variable, then set it as the only def
        data.active_defs -= var_defs[v.index];
        data.active_defs.set(def.pass_index);
        var_defs[v.index].set(def.pass_index);
        data.defined_vars.set(v.index);
      }

      J_A(ND,AI) reaching_defs_visitor(j::mem::bump_pool & pool, u32_t num_defs, u32_t num_uses, u32_t num_vars, bool enable_debug) noexcept
        : pool(pool),
          num_uses(num_uses),
          num_defs(num_defs),
          uses(containers::uninitialized, num_uses),
          defs(containers::uninitialized, num_defs),
          var_defs(containers::uninitialized, num_vars),
          root_data{
            .exposed_uses{num_uses},
            .active_defs{num_defs},
            .defined_vars{num_vars},
          },
          enable_debug(enable_debug)
      {
        for (u32_t i = 0U; i < num_vars; ++i) {
          var_defs.initialize_element(num_defs);
        }
      }

      void write_use_defs(var_data & var_rec, bool is_use = true) noexcept {
        i32_t sz = var_rec.accesses.size();
        debug("Writing {} active {} for var #{} used by {}", sz, is_use ? "defs" : "uses", var_rec.pass_index, *var_rec.expr);
        expr ** target_list = sz ? (expr**)pool.allocate_aligned_zero(J_PTR_SZ * sz, 8U) : nullptr;
        i32_t wr_idx = 0U;
        for (u32_t target_idx : var_rec.accesses) {
          auto & target = (is_use ? defs : uses).at(target_idx);
          if (is_use) {
            target.accesses.set(var_rec.pass_index);
          }
          target_list[wr_idx++] = target.expr;
          debug("  {} #{}: {}", is_use ? "Use" : "Def", wr_idx, *target.expr);
        }
        J_ASSERT(wr_idx == sz);
        *add_bytes<span<expr*>*>(var_rec.expr->static_begin(), 8) = { target_list, sz };
      }

      void visit_short_circuit(expr * J_NOT_NULL e, branch_data & data) noexcept {
          auto g = debug_enter(e->type);
          visit(e->input(0).expr, data);
          for (u32_t i = 1U; i < e->num_inputs; ++i) {
            g.print("Next branch of");
            branch_data d(data);
            visit(e->input(i).expr, d);
            data.add_optional_successor(d);
          }
      }

      void visit_if(expr * J_NOT_NULL e, branch_data & data) noexcept {
          visit(e->input(0).expr, data);
          auto g = debug_enter(e->type);
          branch_data true_branch(data);
          visit(e->input(1).expr, true_branch);
          g.print("Entering else branch in");
          visit(e->input(2).expr, data);
          data.add_alternative_branch(true_branch);
      }

      void visit_repeated_scope(expr * J_NOT_NULL e, branch_data & data) noexcept {
        auto g = debug_enter(e->type);
        branch_data loop_branch(data);
        loop_branch.defined_vars.clear();
        visit(e->input(e->type == expr_type::do_until ? 0U : 1U).expr, loop_branch);

        data.defined_vars |= loop_branch.defined_vars;
        data.active_defs = loop_branch.active_defs;

        loop_branch.exposed_uses -= data.exposed_uses;
        for (auto use_idx : loop_branch.exposed_uses) {
          auto & use = uses.at(use_idx);
          debug("Handling exposed use", use_idx, use.pass_index, use.air_var_index);
          use.accesses |= loop_branch.active_defs;
          use.accesses &= var_defs[use.air_var_index];
          write_use_defs(use);
        }
      }

      void visit(expr * J_NOT_NULL e, branch_data & data) noexcept {
      again:
        switch (e->type) {
        case expr_type::lex_rd: case expr_type::lex_rd_bound:
          on_use(e, data);
          break;

        case expr_type::lex_wr: case expr_type::lex_wr_bound:
          visit(e->input(0).expr, data);
          on_def(e, data);
          break;

        case expr_type::b_if:
          visit_if(e, data);
          break;

        case expr_type::closure:
          visit(e->input(0).expr, data);
          [[fallthrough]];
        case expr_type::do_until:
          visit_repeated_scope(e, data);
          break;


        case expr_type::lor: case expr_type::land:
          visit_short_circuit(e, data);
          break;

        // Single-valued exprs
        case expr_type::sym_val_wr:
        case expr_type::ineg: case expr_type::lnot:
        case expr_type::tag: case expr_type::untag:
        case expr_type::to_bool: case expr_type::as_vec: case expr_type::as_range:
        case expr_type::type_assert: case expr_type::dyn_type_assert:
        case expr_type::make_vec: case expr_type::range_copy: case expr_type::range_length:
          e = e->input(0).expr;
          goto again;

        // Multi-valued exprs
        default:
          for (auto & in : e->inputs()) {
            visit(in.expr, data);
          }
          break;

        case expr_type::sym_val_rd: case expr_type::fn_arg: case expr_type::fn_rest_arg:
        case expr_type::fn_sc_arg: case expr_type::ld_const: case expr_type::nop:
          break;


        case expr_type::act_rec_alloc: case expr_type::act_rec_parent:
        case expr_type::act_rec_rd: case expr_type::act_rec_wr:
          J_FAIL("Act Rec instructions met in reaching defs");
        case expr_type::none: J_UNREACHABLE();
        }
      }

      void execute(expr * J_NOT_NULL e) noexcept {
        visit(e, root_data);
        for (auto & def : defs) {
          write_use_defs(def, false);
        }
      }
    };

    void apply_reaching_defs(air_pass_context & ctx) noexcept {
      reaching_defs_visitor v(
        *ctx.pool,
        ctx.fn->counts[expr_type::lex_wr] + ctx.fn->counts[expr_type::lex_wr_bound],
        ctx.fn->counts[expr_type::lex_rd] + ctx.fn->counts[expr_type::lex_rd_bound],
        ctx.fn->num_lex_vars,
        ctx.print_debug);
      v.execute(ctx.fn->body);
    }
  }

  const air_pass reaching_defs_pass{apply_reaching_defs, "reaching-defs", "Reaching definitions", true};
}
