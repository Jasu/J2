#include "os/dwarf_reader.hpp"
#include "os/dwarf/die_fns.hpp"
#include "containers/hash_map.hpp"
#include "os/dwarf/die_ctx.hpp"
#include "os/dwarf_forms.hpp"
#include "os/dwarf_tag.hpp"
#include "os/dwarf/file_ctx.hpp"
#include "os/dwarf/unit_ctx.hpp"

namespace j::os::dwarf {
  namespace s = strings;

  struct J_AT(HIDDEN) dwarf_context {
    dwarf_context(s::const_string_view main_unit_path)
      : main_reader(main_unit_path)
    {
    }

    def_base * add_def(const char * at, def_base * J_AA(NN) def) {
      J_ASSERT(at);
      auto p1 = def_set.insert(def);
      if (!p1.second) {
        J_ASSERT(*p1.first && *p1.first != def);
        ::delete def;
        def = *p1.first;
        // J_DEBUG("Dupe");
      }
      J_ASSERT(def);
      auto p = defs.insert(at, def);
      J_ASSERT(p.second);
      J_ASSERT(defs.contains(at), "Inserted item does not exist: {:08X} Sz: {}/{} H:{:08X}", (void*)at, defs.size(), defs.capacity(), (u32_t)p.first->hash);
      return def;
    }

    void add_redir(const char * at, const char *to) {
      redirs[at] = to;
    }

    void resolve_defs() {
      for (auto * def : def_set) {
        J_ASSERT(def);
        def->resolve(this);
        resolved_def_set.insert(def);
      }
    }

    void resolve(dwarf_ref & target) {
      if (target.ptr) {
        J_ASSERT(!target.ref);
        return;
      }
      if (target.ref) {
        i32_t num_redirs = 0;
        while (const char * const * re = redirs.maybe_at(target.ref)) {
          num_redirs++;
          target.ref = *re;
          J_ASSERT(*re);
        }
        if (!defs.contains(target.ref)) {
          i32_t i = 0;
          for (auto & p : defs) {
            J_ASSERT(p.first != target.ref);
            ++i;
          }
          J_ASSERT(i == defs.size());
        }
        J_ASSERT(defs.contains(target.ref), "Ref 0x{:06X} not found (redirs: {})",  target.ref - main_reader.debug_info.begin, num_redirs, defs.size());
        def_base * def = defs.at(target.ref);
        J_ASSERT(def);
        def->resolve(this);
        target.ptr = *resolved_def_set.emplace(def).first;
        target.ref = nullptr;
      }
    }

    void resolve_parent(def_child_base *& target) {
      if (target) {
        target->resolve(this);
        target = dynamic_cast<def_child_base*>(*resolved_def_set.emplace(target).first);
      }
    }

    hash_map<const char*, def_base*> defs;
    hash_map<const char*, const char*> redirs;
    hash_set<def_base*, def_hash, def_hash> def_set;
    hash_set<def_base*, def_hash, def_hash> resolved_def_set;
    file_ctx main_reader;
    vector<dwarf_unit_info> units;
  };

  namespace {

  struct J_AT(HIDDEN) debug_info_reader final {
    J_A(AI,HIDDEN,ND) debug_info_reader(dwarf_context * J_AA(NN) ctx, file_ctx * J_AA(NN) file_ctx) noexcept
      : file_reader(file_ctx),
        ctx(ctx)
    {
    }

    i32_t read_array_size() {
      if (!die_rd.has_children()) {
        return -1;
      }
      die_ctx ctx;
      die_rd.enter_children(&ctx);
      i32_t result = -1;
      while (u32_t tag = die_rd.next()) {
        if (tag == dw_tag_subrange_type) {
          die_rd.read_attrs({{dw_at_count, &result}});
        } else {
          die_rd.skip_attrs();
        }
        die_rd.skip_children();
      }
      return result;
    }

    struct J_AT(HIDDEN) param_rec final {
      fn_param param{};
      param_rec * next = nullptr;
    };

    fn_params read_fn_args() {
      if (!die_rd.has_children()) {
        return {};
      }
      die_ctx ctx;
      die_reader rd(die_rd);
      rd.enter_children(&ctx);
      param_rec *first_param = nullptr, **cur_param = &first_param;
      i32_t num_params = 0;
      while (u32_t tag = rd.next()) {
        if (tag == dw_tag_formal_parameter) {
          ++num_params;
          *cur_param = J_ALLOCA_NEW(param_rec){};
          rd.read_attrs({{dw_at_type, &(*cur_param)->param.type}, {dw_at_artificial, &(*cur_param)->param.is_synthetic}});
          cur_param = &(*cur_param)->next;
        } else {
          rd.skip_attrs();
        }
        rd.skip_children();
      }
      fn_params params(num_params);
      for (; first_param; first_param = first_param->next) {
        params.push_back(first_param->param);
      }
      return (fn_params&&)params;
    }

    const char *read_spec_name(dwarf_ref spec) {
      die_reader rd;
      die_ctx spec_ctx;
      rd.open(&spec_ctx, unit, spec.ref);
      J_REQUIRE(rd.next(), "Entering spec failed");
      const char *name = nullptr;
      rd.read_attrs({{dw_at_name, &name}});
      return name;
    }

    void read_fn(def_child_base *parent) {
      const char * name = nullptr;
      bool is_decl = false;
      dwarf_ref return_type;
      dwarf_ref specification;
      dwarf_ref abstract_origin;
      die_rd.read_attrs({
          {dw_at_name, &name},
          {dw_at_abstract_origin, &abstract_origin},
          {dw_at_type, &return_type},
          {dw_at_specification, &specification},
          {dw_at_declaration, &is_decl}});
      const char *offset = die_rd.die_begin();
      if (abstract_origin) {
        ctx->add_redir(offset, abstract_origin.ref);
        die_rd.skip_children();
        return;
      } else if (specification) {
        ctx->add_redir(offset, specification.ref);
      } else {
        parent = static_cast<def_child_base*>(ctx->add_def(offset, ::new def_fn_decl(name, parent, {return_type, read_fn_args()})));
      }
      read_children(parent);
    }

    J_A(NODISC) bool read_child(def_child_base *parent) {
      const char * offset = die_rd.ir.offset;
      u32_t tag = die_rd.next();
      if (!tag) {
        return false;
      }
      def_base *cur = nullptr;
      bool descend = false;
      const char * name = nullptr;
      bool inline_ns = false;
      dwarf_ref target;
      dwarf_ref containing;
      attr_spec args[]{{dw_at_name, &name}, {dw_at_type, &target}, {dw_at_export_syms, &inline_ns}, {dw_at_cont_type, &containing}};
      switch (tag) {
      case dw_tag_subprogram:
        read_fn(parent);
        return true;
      case dw_tag_subroutine_type:
        die_rd.read_attrs(args);
        cur = ::new def_fn_type({target, read_fn_args()});
        break;
      case dw_tag_enumeration_type:
        die_rd.read_attrs(args);
        cur = ::new def_enum_type(name, parent, false);
        break;
      case dw_tag_ptr_to_member_type:
        die_rd.read_attrs(args);
        cur = ::new def_mem_ptr_type(target, containing);
        break;
      case dw_tag_union_type:
        die_rd.read_attrs(args);
        cur = ::new def_class_type(name, parent, dt_union);
        descend = true;
        break;
      case dw_tag_class_type:
        die_rd.read_attrs(args);
        cur = ::new def_class_type(name, parent, dt_class);
        descend = true;
        break;
      case dw_tag_structure_type:
        die_rd.read_attrs(args);
        cur = ::new def_class_type(name, parent, dt_struct);
        descend = true;
        break;
      case dw_tag_base_type:
      case dw_tag_unspecified_type:
        die_rd.read_attrs(args);
        cur = ::new def_base_type(name);
        break;
      case dw_tag_typedef:
        die_rd.read_attrs(args);
        cur = ::new def_typedef(name, parent, target);
        break;
      case dw_tag_pointer_type:
        die_rd.read_attrs(args);
        cur = ::new def_wrapped_type(target, dt_ptr_type);
        break;
      case dw_tag_reference_type:
        die_rd.read_attrs(args);
        cur = ::new def_wrapped_type(target, dt_ref_type);
        break;
      case dw_tag_rvalue_reference_type:
        die_rd.read_attrs(args);
        cur = ::new def_wrapped_type(target, dt_rref_type);
        break;
      case dw_tag_const_type:
        die_rd.read_attrs(args);
        cur = ::new def_wrapped_type(target, dt_const_type);
        break;
      case dw_tag_restrict_type:
        die_rd.read_attrs(args);
        cur = ::new def_wrapped_type(target, dt_restrict_type);
        break;
      case dw_tag_array_type:
        die_rd.read_attrs(args);
        cur = ::new def_array_type(target, read_array_size());
        break;
      case dw_tag_namespace:
        die_rd.read_attrs(args);
        cur = ::new def_ns(name, parent, inline_ns);
        descend = true;
        break;
      default:
        // child_reader.dump_die(true);
        die_rd.skip_attrs();
        die_rd.skip_children();
        return true;
      }
      // child_reader.dump_die(false);
      // cur->dump_full_name();
      cur = ctx->add_def(offset, cur);

      if (descend) {
        read_children(static_cast<def_child_base*>(cur));
      } else if (tag != dw_tag_array_type) {
        die_rd.skip_children();
      }
      return true;
    }

      void read_children(def_child_base *parent_def) {
        if (!die_rd.has_children()) {
          return;
        }
        die_ctx die;
        die_rd.enter_children(&die);
        while (read_child(parent_def)) { }
      }

      bool read_unit() {
        if (unit && unit->unit_end() > file_reader->debug_info.end - 12) {
          return false;
        }
        unit = unit_ctx::open_unit(file_reader, unit ? unit->unit_end() : nullptr);
        if (!*unit) {
          J_DEBUG("Unit not OK");
        }
        die_ctx unit_ctx;
        die_rd.open(&unit_ctx, unit);
        if (J_UNLIKELY(!die_rd.next())) {
          return false;
        }
        die_rd.skip_attrs();
        read_children(nullptr);
        return true;
      }

      void read_units() {
        while (read_unit()) { }
      }

      die_reader die_rd;
      unit_ctx * unit = nullptr;
      file_ctx * file_reader;
      dwarf_context * ctx;
    };
  }

  J_A(NODISC) dwarf_info dwarf_reader::read() {
    J_ASSERT(!ctx);
    ctx = ::new dwarf_context(path);
    debug_info_reader main_di_reader{ctx, &ctx->main_reader};
    main_di_reader.read_units();
    ctx->resolve_defs();
    for (auto * def : ctx->resolved_def_set) {
      if (def->type == dt_fn_decl) {
        def->dump_name(nullptr);
        J_DEBUG("");
      }
    }

    return {
      static_cast<vector<dwarf_unit_info> &&>(ctx->units),
    };
  }

  dwarf_reader::~dwarf_reader() {
    ::delete ctx;
  }

  void resolve_ref(dwarf_context * J_AA(NN) ctx, dwarf_ref & ref) noexcept {
    ctx->resolve(ref);
  }

  void resolve_parent(dwarf_context * J_AA(NN) ctx, def_child_base * & ref) noexcept {
    ctx->resolve_parent(ref);
  }
}
