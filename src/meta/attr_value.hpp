#pragma once

#include "strings/string_map_fwd.hpp"
#include "meta/basic_node_set.hpp"
#include "containers/span.hpp"
#include "containers/pair.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "strings/string.hpp"
#include "lisp/common/id.hpp"

namespace j::meta {
  struct codegen_template;
  struct term;
  struct struct_def;
  struct enum_entry;
  struct enum_def;
  struct node;
  struct term;
  struct attr_def;
  struct term_member;
  struct val_type;

  using attr_value_array = trivial_array_copyable<attr_value>;

  enum attr_type : u8_t {
    attr_type_none = 0,
    attr_int,
    attr_bool,
    attr_enum,

    attr_term,
    attr_term_member,
    attr_value_type,
    attr_attr_def,
    attr_enum_def,
    attr_struct_def,
    attr_gen_file_set,
    attr_nonterminal,
    attr_fn,

    attr_term_set,
    attr_val_type_set,

    attr_rule,

    attr_id,
    attr_str,
    attr_tpl,
    attr_tuple,
    attr_map,
    attr_struct,

    attr_max = attr_struct,
  };

  constexpr inline u8_t num_attr_types_v J_A(ND) = attr_max;

  enum J_A(FLAGS) attr_type_mask : u32_t {
    attr_mask_none             = 0b000000000000000000000U,

    attr_mask_int              = 0b000000000000000000001U,
    attr_mask_bool             = 0b000000000000000000010U,
    attr_mask_enum             = 0b000000000000000000100U,
    attr_mask_int_or_enum      = 0b000000000000000000101U,

    attr_mask_term             = 0b000000000000000001000U,
    attr_mask_term_member      = 0b000000000000000010000U,
    attr_mask_value_type       = 0b000000000000000100000U,
    attr_mask_attr_def         = 0b000000000000001000000U,
    attr_mask_enum_def         = 0b000000000000010000000U,
    attr_mask_struct_def       = 0b000000000000100000000U,
    attr_mask_gen_file_set     = 0b000000000001000000000U,
    attr_mask_nonterminal      = 0b000000000010000000000U,
    attr_mask_fn               = 0b000000000100000000000U,
    attr_mask_node             = 0b000000000111111111000U,

    attr_mask_term_set         = 0b000000001000000000000U,
    attr_mask_val_type_set     = 0b000000010000000000000U,
    attr_mask_node_set         = 0b000000011000000000000U,

    attr_mask_rule             = 0b000000100000000000000U,
    attr_mask_id               = 0b000001000000000000000U,
    attr_mask_str              = 0b000010000000000000000U,
    attr_mask_tpl              = 0b000100000000000000000U,
    attr_mask_tuple            = 0b001000000000000000000U,
    attr_mask_map              = 0b010000000000000000000U,
    attr_mask_struct           = 0b100000000000000000000U,

    attr_mask_collection       = 0b011000011000000000000U,

    attr_mask_any              = 0b111111111111111111111U,
  };

  struct attr_value;

  struct struct_val final {
    attr_value_array * values = nullptr;
    const struct_def * def = nullptr;

    [[nodiscard]] const attr_value & at(strings::const_string_view name) const;
    void set(strings::const_string_view name, attr_value && value);
    void set(strings::const_string_view name, const attr_value & value);
    [[nodiscard]] bool has(strings::const_string_view name) const noexcept;
  };

  using attr_map_t = strings::string_map<attr_value>;

  constexpr inline u8_t attr_mask_width_v J_A(ND) = num_attr_types_v;

  template<typename T> extern const attr_type_mask mask_v J_A(ND,HIDDEN);

  template<> inline constexpr const attr_type_mask mask_v<bool> J_A(ND,HIDDEN) = attr_mask_bool;

  template<> inline constexpr const attr_type_mask mask_v<term> J_A(ND,HIDDEN) = attr_mask_term;
  template<> inline constexpr const attr_type_mask mask_v<term_member> J_A(ND,HIDDEN) = attr_mask_term_member;
  template<> inline constexpr const attr_type_mask mask_v<val_type> J_A(ND,HIDDEN) = attr_mask_value_type;
  template<> inline constexpr const attr_type_mask mask_v<attr_def> J_A(ND,HIDDEN) = attr_mask_attr_def;
  template<> inline constexpr const attr_type_mask mask_v<enum_def> J_A(ND,HIDDEN) = attr_mask_enum_def;
  template<> inline constexpr const attr_type_mask mask_v<struct_def> J_A(ND,HIDDEN) = attr_mask_struct_def;
  template<> inline constexpr const attr_type_mask mask_v<node> J_A(ND,HIDDEN) = attr_mask_node;
  template<> inline constexpr const attr_type_mask mask_v<fn> J_A(ND,HIDDEN) = attr_mask_fn;

  template<> inline constexpr const attr_type_mask mask_v<lisp::id> J_A(ND,HIDDEN) = attr_mask_id;
  template<> inline constexpr const attr_type_mask mask_v<lisp::id_name> J_A(ND,HIDDEN) = attr_mask_id;
  template<> inline constexpr const attr_type_mask mask_v<lisp::id_name_ref> J_A(ND,HIDDEN) = attr_mask_id;
  template<> inline constexpr const attr_type_mask mask_v<rule> J_A(ND,HIDDEN) = attr_mask_rule;
  template<> inline constexpr const attr_type_mask mask_v<codegen_template> J_A(ND,HIDDEN) = attr_mask_tpl;
  template<> inline constexpr const attr_type_mask mask_v<strings::string> J_A(ND,HIDDEN) = attr_mask_str;
  template<> inline constexpr const attr_type_mask mask_v<span<const attr_value>> J_A(ND,HIDDEN) = attr_mask_tuple;
  template<> inline constexpr const attr_type_mask mask_v<attr_value> J_A(ND,HIDDEN) = attr_mask_any;

  template<ActuallyIntegral T> inline constexpr const attr_type_mask mask_v<T> J_A(ND,HIDDEN) = attr_mask_int;

  template<typename... Ts>
  J_A(AI,ND,HIDDEN) constexpr inline attr_type_mask to_attr_mask(Ts... attr_types) noexcept {
    return (attr_type_mask)(0U | ... | (1U << ((31 & attr_types) - 1U)));
  }

  J_A(AI,ND,HIDDEN,NODISC) inline node_type node_set_type(attr_type t) noexcept {
    switch (t) {
    case attr_term_set: return node_term;
    case attr_val_type_set: return node_value_type;
    default: return node_none;
    }
  }

  J_A(AI,ND,HIDDEN,NODISC) inline attr_type node_set_node_attr_type(attr_type t) noexcept {
    switch (t) {
    case attr_term_set: return attr_term;
    case attr_val_type_set: return attr_value_type;
    default: return attr_type_none;
    }
  }

  J_A(AI,ND,HIDDEN,NODISC) inline attr_type_mask node_set_attr_mask(attr_type t) noexcept {
    switch (t) {
    case attr_term_set: return attr_mask_term;
    case attr_val_type_set: return attr_mask_value_type;
    default: return attr_mask_none;
    }
  }

  struct attr_value final {
    J_A(AI,ND) inline explicit attr_value(bool boolean) noexcept
      : type(attr_bool),
        integer(boolean)
    { }

    template<ActuallyIntegral T>
    J_A(AI,ND) inline explicit attr_value(T integer) noexcept
      : type(attr_int),
        integer(integer)
    { }

    template<ActuallyIntegral T>
    J_A(AI,ND) inline attr_value(const enum_def * J_NOT_NULL def, T integer) noexcept
      : type(attr_enum),
        enum_val{integer, def}
    { }

    explicit attr_value(node * J_NOT_NULL n) noexcept;

    J_A(AI,ND) inline explicit attr_value(struct rule * J_NOT_NULL r) noexcept
    : type(attr_rule),
      rule(r)
    { }

    explicit attr_value(const char * J_NOT_NULL str) noexcept;

    explicit attr_value(strings::const_string_view str) noexcept;

    explicit attr_value(lisp::id_name && id) noexcept;

    explicit attr_value(lisp::id_name_ref id) noexcept;

    J_A(AI,ND) inline explicit attr_value(strings::string && str) noexcept
      : type(attr_str),
        string(static_cast<strings::string &&>(str))
    { }


    explicit attr_value(const attr_map_t & map) noexcept;

    explicit attr_value(attr_map_t && map) noexcept;

    explicit attr_value(const codegen_template & tpl) noexcept;

    explicit attr_value(codegen_template && tpl) noexcept;

    explicit attr_value(attr_value_array && values) noexcept;

    explicit attr_value(span<attr_value> values) noexcept;
    explicit attr_value(struct_def * J_NOT_NULL def, span<pair<const char*, attr_value>> values) noexcept;

    explicit attr_value(attr_type t, const basic_node_set & val) noexcept;

    J_A(AI,ND) inline attr_value() noexcept : integer(0) { }

    J_A(AI,ND) inline attr_value(attr_value && rhs) noexcept {
      j::memcpy(this, &rhs, sizeof(attr_value));
      rhs.type = attr_type_none;
    }

    attr_value(const attr_value & rhs) noexcept;

    attr_value & operator=(attr_value && rhs) noexcept;

    attr_value & operator=(const attr_value & rhs) noexcept;

    void clear() noexcept;

    J_A(AI) inline ~attr_value() {
      if (type >= attr_id) {
        clear();
      }
    }

    void assert_type_mask(attr_type_mask mask) const;

    J_A(AI,NODISC) inline bool empty() const noexcept { return type == attr_type_none; }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return type != attr_type_none; }
    J_A(AI,NODISC) inline bool operator!() const noexcept { return type == attr_type_none; }

    J_A(NODISC) bool value_empty() const noexcept;

    J_A(AI,ND,NODISC,HIDDEN) inline bool is_int()         const noexcept { return type == attr_int; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_bool()        const noexcept { return type == attr_bool; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool is_value_type()  const noexcept { return type == attr_value_type; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_attr_def()    const noexcept { return type == attr_attr_def; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_enum_def()    const noexcept { return type == attr_enum_def; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool is_enum()        const noexcept { return type == attr_enum; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_str()         const noexcept { return type == attr_str; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_id()         const noexcept { return type == attr_id; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_tpl()         const noexcept { return type == attr_tpl; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_tuple()       const noexcept { return type == attr_tuple; }
    J_A(AI,ND,NODISC) inline bool is_node()        const noexcept { return to_attr_mask(type) & attr_mask_node; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_term()        const noexcept { return type == attr_term; }
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_val_type()        const noexcept { return type == attr_value_type; }

    J_A(AI,ND,NODISC) inline bool is_node_set()    const noexcept { return to_attr_mask(type) & attr_mask_node_set; }


    J_A(ND,NODISC) inline i64_t as_int() const noexcept {
      assert_type_mask(attr_mask_int);
      return integer;
    }

    J_A(ND,NODISC) inline const strings::string & as_str() const noexcept {
      assert_type_mask(attr_mask_str);
      return string;
    }

    J_A(ND,NODISC) inline const codegen_template & as_tpl() const noexcept {
      assert_type_mask(attr_mask_tpl);
      return *tpl;
    }

    J_A(ND,NODISC) inline bool as_bool() const noexcept {
      assert_type_mask(attr_mask_bool);
      return integer;
    }

    J_A(ND,NODISC) inline span<const attr_value> as_tuple() const noexcept {
      assert_type_mask(attr_mask_tuple);
      return *tuple;
    }

    J_A(ND,NODISC) inline node & as_node() const noexcept {
      assert_type_mask(attr_mask_node);
      return *node;
    }

    [[nodiscard]] inline term & as_term() const noexcept {
      assert_type_mask(attr_mask_term);
      return *reinterpret_cast<term*>(node);
    }

    [[nodiscard]] inline nonterminal & as_nonterminal() const noexcept {
      assert_type_mask(attr_mask_nonterminal);
      return *reinterpret_cast<nonterminal*>(node);
    }

    [[nodiscard]] inline attr_def & as_attr_def() const noexcept {
      assert_type_mask(attr_mask_attr_def);
      return *reinterpret_cast<attr_def*>(node);
    }

    [[nodiscard]] inline term_member & as_term_member() const noexcept {
      assert_type_mask(attr_mask_term_member);
      return *reinterpret_cast<term_member*>(node);
    }

    [[nodiscard]] inline val_type & as_val_type() const noexcept {
      assert_type_mask(attr_mask_value_type);
      return *reinterpret_cast<val_type*>(node);
    }

    [[nodiscard]] inline generated_file_set & as_gen_file_set() const noexcept {
      assert_type_mask(attr_mask_gen_file_set);
      return *reinterpret_cast<generated_file_set*>(node);
    }

    [[nodiscard]] inline fn & as_fn() const noexcept {
      assert_type_mask(attr_mask_fn);
      return *reinterpret_cast<fn*>(node);
    }

    [[nodiscard]] inline enum_def & as_enum_def() const noexcept {
      assert_type_mask(attr_mask_enum_def);
      return *reinterpret_cast<enum_def*>(node);
    }

    [[nodiscard]] u64_t as_enum() const noexcept {
      assert_type_mask(attr_mask_enum);
      return enum_val.value;
    }

    [[nodiscard]] wrapped_node_set as_node_set() const noexcept {
      assert_type_mask(attr_mask_node_set);
      return wrapped_node_set{&node_set, node_set_type(type)};
    }

    [[nodiscard]] const enum_def & get_enum_def() const noexcept {
      assert_type_mask(attr_mask_enum);
      return *enum_val.def;
    }

    template<typename T>
    [[nodiscard]] inline T as() noexcept;

    template<typename T>
    [[nodiscard]] inline T as() const noexcept;

    [[nodiscard]] bool operator==(const attr_value & rhs) const noexcept;
    [[nodiscard]] bool operator<(const attr_value & rhs) const noexcept;

    attr_type type = attr_type_none;
    union {
      i64_t integer;
      struct {
        u64_t value;
        const enum_def * def;
      } enum_val;
      basic_node_set node_set;
      strings::string string;
      struct {
        lisp::id_name name;
        lisp::id resolved;
      } id;
      codegen_template * tpl;
      node * node;
      rule * rule;
      struct_val struct_val;
      attr_value_array * tuple;
      attr_map_t * map;
    };
  };

  template<typename T>
  [[nodiscard]] inline T attr_value::as() noexcept {
    return const_cast<const attr_value *>(this)->as<T>();
  }

  template<> [[nodiscard]] inline const lisp::id & attr_value::as<const lisp::id &>() const noexcept {
    assert_type_mask(attr_mask_id);
    J_ASSERT(id.resolved);
    return id.resolved;
  }

  template<> [[nodiscard]] inline lisp::id & attr_value::as<lisp::id &>() const noexcept {
    assert_type_mask(attr_mask_id);
    J_ASSERT(id.resolved);
    return const_cast<lisp::id &>(id.resolved);
  }

  template<> [[nodiscard]] inline lisp::id_name & attr_value::as<lisp::id_name &>() const noexcept {
    assert_type_mask(attr_mask_id);
    return const_cast<lisp::id_name &>(id.name);
  }

  template<> [[nodiscard]] inline const lisp::id_name & attr_value::as<const lisp::id_name &>() const noexcept {
    assert_type_mask(attr_mask_id);
    return id.name;
  }

  template<> [[nodiscard]] inline node & attr_value::as<node &>() const noexcept {
    assert_type_mask(attr_mask_node);
    return *node;
  }

  template<> [[nodiscard]] inline term & attr_value::as<term &>() const noexcept {
    assert_type_mask(attr_mask_term);
    return *reinterpret_cast<term*>(node);
  }

  template<> [[nodiscard]] inline attr_map_t & attr_value::as<attr_map_t &>() const noexcept {
    assert_type_mask(attr_mask_map);
    return *reinterpret_cast<attr_map_t*>(map);
  }

  template<> [[nodiscard]] inline term_member & attr_value::as<term_member &>() const noexcept {
    assert_type_mask(attr_mask_term_member);
    return *reinterpret_cast<term_member*>(node);
  }

  template<> [[nodiscard]] inline val_type & attr_value::as<val_type &>() const noexcept {
    assert_type_mask(attr_mask_value_type);
    return *reinterpret_cast<val_type*>(node);
  }

  template<> [[nodiscard]] inline attr_def & attr_value::as<attr_def &>() const noexcept {
    assert_type_mask(attr_mask_attr_def);
    return *reinterpret_cast<attr_def*>(node);
  }

  template<> [[nodiscard]] inline enum_def & attr_value::as<enum_def &>() const noexcept {
    assert_type_mask(attr_mask_enum_def);
    return *reinterpret_cast<enum_def*>(node);
  }

  template<> [[nodiscard]] inline struct rule & attr_value::as<rule &>() const noexcept {
    assert_type_mask(attr_mask_rule);
    return *rule;
  }

  template<> [[nodiscard]] inline nonterminal & attr_value::as<nonterminal &>() const noexcept {
    assert_type_mask(attr_mask_nonterminal);
    return *reinterpret_cast<nonterminal*>(node);
  }

  template<> [[nodiscard]] inline i64_t attr_value::as<i64_t>() const noexcept {
    assert_type_mask(attr_mask_int);
    return integer;
  }

  template<> [[nodiscard]] inline const strings::string & attr_value::as<const strings::string &>() const noexcept {
    assert_type_mask(attr_mask_str);
    return string;
  }

  template<> [[nodiscard]] inline strings::string attr_value::as<strings::string>() const noexcept {
    assert_type_mask(attr_mask_str);
    return string;
  }

  template<> [[nodiscard]] inline strings::const_string_view attr_value::as<strings::const_string_view>() const noexcept {
    assert_type_mask(attr_mask_str);
    return string;
  }
  template<> [[nodiscard]] inline const codegen_template & attr_value::as<const codegen_template &>() const noexcept {
    assert_type_mask(attr_mask_tpl);
    return *tpl;
  }

  template<> J_A(AI,NODISC) inline bool attr_value::as<bool>() const noexcept {
    assert_type_mask(attr_mask_bool);
    return integer;
  }

  template<> [[nodiscard]] inline span<const attr_value> attr_value::as<span<const attr_value>>() const noexcept {
    assert_type_mask(attr_mask_tuple);
    return *tuple;
  }

  template<> [[nodiscard]] inline const attr_map_t & attr_value::as<const attr_map_t &>() const noexcept {
    assert_type_mask(attr_mask_map);
    return *map;
  }

  template<> [[nodiscard]] inline const attr_value_array & attr_value::as<const attr_value_array &>() const noexcept {
    assert_type_mask(attr_mask_tuple);
    return *tuple;
  }

  template<> [[nodiscard]] inline const trivial_array<attr_value> & attr_value::as<const trivial_array<attr_value> &>() const noexcept {
    assert_type_mask(attr_mask_tuple);
    return *tuple;
  }


  template<> [[nodiscard]] inline const node & attr_value::as<const node &>() const noexcept {
    assert_type_mask(attr_mask_node);
    return *node;
  }

  template<> [[nodiscard]] inline const term & attr_value::as<const term &>() const noexcept {
    assert_type_mask(attr_mask_term);
    return *reinterpret_cast<term*>(node);
  }

  template<> [[nodiscard]] inline const term_member & attr_value::as<const term_member &>() const noexcept {
    assert_type_mask(attr_mask_term_member);
    return *reinterpret_cast<term_member*>(node);
  }

  template<> [[nodiscard]] inline const val_type & attr_value::as<const val_type &>() const noexcept {
    assert_type_mask(attr_mask_value_type);
    return *reinterpret_cast<val_type*>(node);
  }

  template<> [[nodiscard]] inline const attr_def & attr_value::as<const attr_def &>() const noexcept {
    assert_type_mask(attr_mask_attr_def);
    return *reinterpret_cast<attr_def*>(node);
  }

  template<> [[nodiscard]] inline const enum_def & attr_value::as<const enum_def &>() const noexcept {
    assert_type_mask(attr_mask_enum_def);
    return *reinterpret_cast<enum_def*>(node);
  }

  template<> [[nodiscard]] inline const struct rule & attr_value::as<const rule &>() const noexcept {
    assert_type_mask(attr_mask_rule);
    return *reinterpret_cast<struct rule*>(node);
  }

  template<> [[nodiscard]] inline const nonterminal & attr_value::as<const nonterminal &>() const noexcept {
    assert_type_mask(attr_mask_nonterminal);
    return *reinterpret_cast<nonterminal*>(node);
  }

  template<> [[nodiscard]] inline const basic_node_set & attr_value::as<const basic_node_set &>() const noexcept {
    assert_type_mask(attr_mask_node_set);
    return node_set;
  }
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::meta::attr_value);
J_DECLARE_EXTERN_STRING_MAP(j::meta::attr_value);
