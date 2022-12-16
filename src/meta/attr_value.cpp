#include "meta/attr_value.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/rules/rule.hpp"
#include "strings/string_map.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/node.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::meta::attr_value);

namespace j::meta {
  namespace {
    void copy_attr_value(attr_value * to, const attr_value & from) {
      to->type = from.type;
      switch (from.type) {
      case attr_type_none:
        break;
      case attr_term:
      case attr_term_member:
      case attr_value_type:
      case attr_attr_def:
      case attr_struct_def:
      case attr_enum_def:
      case attr_nonterminal:
      case attr_gen_file_set:
      case attr_fn:
        to->node = from.node;
        break;
      case attr_rule:
        to->rule = from.rule;
        break;
      case attr_int:
      case attr_bool:
        to->integer = from.integer;
        break;
      case attr_term_set:
      case attr_val_type_set:
        to->node_set = from.node_set;
        break;
      case attr_enum:
        to->enum_val = from.enum_val;
        break;
      case attr_id:
        ::new (&to->id.name) lisp::id_name(from.id.name);
        to->id.resolved = from.id.resolved;
        break;
      case attr_str:
        ::new (&to->string) strings::string(from.string);
        break;
      case attr_tpl:
        to->tpl = ::new codegen_template(*from.tpl);
        break;
      case attr_struct:
        ::new (&to->struct_val) struct_val{from.struct_val};
        break;
      case attr_tuple:
        to->tuple = ::new trivial_array_copyable<attr_value>(*from.tuple);
        break;
      case attr_map:
        to->map = ::new attr_map_t(*from.map);
        break;
    }
    }
  }

  [[nodiscard]] const attr_value & struct_val::at(strings::const_string_view name) const {
    return values->at(def->index_of(name));
  }

  void struct_val::set(strings::const_string_view name, attr_value && value) {
    values->at(def->index_of(name)) = static_cast<attr_value &&>(value);
  }

  void struct_val::set(strings::const_string_view name, const attr_value & value) {
    values->at(def->index_of(name)) = value;
  }

  [[nodiscard]] bool struct_val::has(strings::const_string_view name) const noexcept {
    if (!def->has(name)) {
      return false;
    }
    return (bool)values->at(def->index_of(name));
  }

    attr_value::attr_value(struct node * J_NOT_NULL n) noexcept
      : node(n)
    {
      switch (n->type) {
      case node_term:
        type = attr_term;
        break;
      case node_fn:
        type = attr_fn;
        break;
      case node_term_member:
        type = attr_term_member;
        break;
      case node_value_type:
        type = attr_value_type;
        break;
      case node_attr_def:
        type = attr_attr_def;
        break;
      case node_enum:
        type = attr_enum_def;
        break;
      case node_struct_def:
        type = attr_struct_def;
        break;
      case node_nonterminal:
        type = attr_nonterminal;
        break;
      case node_gen_file_set:
        type = attr_gen_file_set;
        break;
      case node_any:
      case node_none:
        J_FAIL("Unsupported node type");
      }
    }

  attr_value::attr_value(const char * J_NOT_NULL str) noexcept
      : type(attr_str),
        string(str)
    { }

  void attr_value::clear() noexcept {
    if (type == attr_str) {
      string.~string();
    } else if (type == attr_id) {
      id.name.~id_name();
    } else if (type == attr_tpl) {
      ::delete tpl;
    } else if (type == attr_tuple) {
      ::delete tuple;
    } else if (type == attr_map) {
      ::delete map;
    }
    type = attr_type_none;
  }

  attr_value::attr_value(lisp::id_name && id) noexcept
    : type(attr_id),
      id{static_cast<lisp::id_name &&>(id), lisp::id(nullptr)}
  {
  }


  attr_value::attr_value(lisp::id_name_ref id) noexcept
    : type(attr_id),
      id{(lisp::id_name)id, lisp::id(nullptr)}
  { }

  attr_value::attr_value(strings::const_string_view str) noexcept
      : type(attr_str),
        string(str)
    { }

  attr_value::attr_value(const attr_map_t & map) noexcept
    : type(attr_map),
      map(::new attr_map_t(map))
  {
  }

  attr_value::attr_value(attr_map_t && map) noexcept
    : type(attr_map),
      map(::new attr_map_t(static_cast<attr_map_t &&>(map)))
  {
  }

  attr_value::attr_value(attr_value_array && values) noexcept
    : type(attr_tuple),
      tuple(::new attr_value_array(static_cast<attr_value_array &&>(values)))
  {
  }

  attr_value::attr_value(span<attr_value> rhs) noexcept
    : type(attr_tuple),
      tuple(::new attr_value_array(containers::move, rhs))
  {
  }
  attr_value::attr_value(struct_def * J_NOT_NULL def, span<pair<const char*, attr_value>> values) noexcept
    : type(attr_struct),
      struct_val{.values = ::new attr_value_array(def->size()), .def = def}
  {
    for (auto & p : values) {
      struct_val.set(p.first, p.second);
    }
  }

  attr_value::attr_value(const attr_value & rhs) noexcept {
    copy_attr_value(this, rhs);
  }

  attr_value::attr_value(const codegen_template & tpl) noexcept
    : type(attr_tpl),
      tpl(::new codegen_template(tpl))
  {
  }

  attr_value::attr_value(codegen_template && tpl) noexcept
    : type(attr_tpl),
      tpl(::new codegen_template(static_cast<codegen_template &&>(tpl)))
  {
  }

  attr_value::attr_value(attr_type type, const basic_node_set & val) noexcept
    : type(type),
      node_set(val)
  {
  }

  attr_value & attr_value::operator=(attr_value && rhs) noexcept {
    if (this != &rhs) {
      clear();
      j::memcpy(this, &rhs, sizeof(attr_value));
      rhs.type = attr_type_none;
    }
    return *this;
  }

  attr_value & attr_value::operator=(const attr_value & rhs) noexcept {
    if (this != &rhs) {
      clear();
      copy_attr_value(this, rhs);
    }
    return *this;
  }

  void attr_value::assert_type_mask(attr_type_mask mask) const {
    J_ASSERT(type != attr_type_none, "Empty value, expected {}.", mask);
    J_ASSERT(to_attr_mask(type) & mask, "Expected {}, got {}.", mask, type);
  }

  bool attr_value::value_empty() const noexcept {
    switch (type) {
    case attr_type_none:
      return true;
    case attr_str:
      return !string;
    case attr_map:
      return !*map;
    case attr_tpl:
      return !*tpl;
    case attr_tuple:
      return !tuple || !*tuple;
    case attr_bool:
    case attr_int:
      return !integer;
    case attr_enum:
      return !enum_val.value;
    case attr_term_set:
    case attr_val_type_set:
      return !node_set;
    case attr_id:
    case attr_term:
    case attr_term_member:
    case attr_gen_file_set:
    case attr_value_type:
    case attr_attr_def:
    case attr_enum_def:
    case attr_struct_def:
    case attr_struct:
    case attr_fn:
    case attr_rule:
    case attr_nonterminal:
      return false;
    }
  }

  [[nodiscard]] bool attr_value::operator<(const attr_value & rhs) const noexcept {
    if (type != rhs.type) {
      return type < rhs.type;
    }
    switch (type) {
    case attr_type_none:
      return false;
    case attr_id:
      return id.name.pkg < rhs.id.name.pkg || (id.name.pkg == rhs.id.name.pkg && id.name.name < rhs.id.name.name);
    case attr_str:
      return string < rhs.string;
    case attr_tpl:
      return *tpl < *rhs.tpl;
    case attr_enum:
      if (!enum_val.def != !rhs.enum_val.def) {
        return !enum_val.def > !rhs.enum_val.def;
      }
      if (enum_val.def && enum_val.def != rhs.enum_val.def) {
        return enum_val.def->name < rhs.enum_val.def->name;
      }
      return enum_val.value < rhs.enum_val.value;
    case attr_map:
      if (map->size() != rhs.map->size()) {
        return map->size() < rhs.map->size();
      }
      return false;
    case attr_tuple:
      if (tuple->size() != rhs.tuple->size()) {
        return tuple->size() < rhs.tuple->size();
      }
      for (u32_t i = 0U, e = tuple->size(); i != e; ++i) {
        if ((*tuple)[i] != (*rhs.tuple)[i]) {
          return (*tuple)[i] < (*rhs.tuple)[i];
        }
      }
      return false;
    case attr_term:
    case attr_value_type:
      return node->index < rhs.node->index;
    case attr_attr_def:
    case attr_enum_def:
    case attr_term_member:
      return node->name < rhs.node->name;

    case attr_rule:
      return rule->precedence < rhs.rule->precedence;
    default:
      return integer < rhs.integer;
    }
  }

  [[nodiscard]] bool attr_value::operator==(const attr_value & rhs) const noexcept {
    if (type != rhs.type) {
      return false;
    }
    switch (type) {
    case attr_type_none:
      return true;
    case attr_id:
      return id.name == rhs.id.name;
    case attr_str:
      return string == rhs.string;
    case attr_tpl:
      return *tpl == *rhs.tpl;
    case attr_term_set: case attr_val_type_set:
      return node_set == rhs.node_set;
    case attr_enum:
      return enum_val.value == rhs.enum_val.value && enum_val.def == rhs.enum_val.def;
    case attr_map:
      if (map->size() != rhs.map->size()) {
        return false;
      }
      for (auto & p : *map) {
        if (auto r = rhs.map->maybe_at(p.first)) {
          if (p.second != *r) {
            return false;
          }
        } else {
          return false;
        }
      }
      return true;
    case attr_struct:
      if (struct_val.def != rhs.struct_val.def) {
        return false;
      }
      for (auto & f : struct_val.def->fields) {
        bool has = struct_val.has(f);
        if (has != rhs.struct_val.has(f)) {
          return false;
        }
        if (has && struct_val.at(f) != rhs.struct_val.at(f)) {
          return false;
        }
      }
      return true;

    case attr_tuple:
      if (tuple->size() != rhs.tuple->size()) {
        return false;
      }
      for (u32_t i = 0U, e = tuple->size(); i != e; ++i) {
        if ((*tuple)[i] != (*rhs.tuple)[i]) {
          return false;
        }
      }
      return true;
    default:
      return integer == rhs.integer;
    }
  }
}
