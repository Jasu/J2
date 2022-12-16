#include "serialization/yaml/value_wrapper.hpp"

#include "properties/registration.hpp"
#include "exceptions/exceptions.hpp"
#include "serialization/yaml/csubstr.hpp"
#include "hzd/ctype.hpp"
#include "strings/parsing/parse_int.hpp"
#include "strings/parsing/parse_float.hpp"

// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wextra-semi-stmt"
// #define _C4_CHARCONV_HPP_
#include <c4/yml/tree.hpp>
// #pragma clang diagnostic pop

namespace j::serialization::yaml::detail {
  extern const properties::access::list_access_definition * yaml_list_access_definition;

  namespace {
    namespace p = properties;
    namespace a = p::access;

    /// Throw an exception signifying that mutating YAML data is not (yet) supported.
    ///
    /// \note The function is marked not to be inlined, since throwing an exception
    /// requires lots of code, and function chain leading to the call would likely
    /// consist of a two nearly-empty wrapper functions, which would likely trigger
    /// inlining.
    [[noreturn]] J_HIDDEN J_NO_INLINE void throw_on_mutate() {
      throw j::exceptions::not_implemented_exception()
        << j::exceptions::message("Mutating YAML properties is not implemented.");
    }

    /// Wraps YAML mappings for property access.
    ///
    /// Note that key_type is c4::csubstr, even though the actual type
    /// should be yaml_wrapper_data, like the value. However, since rapidyaml does
    /// not support non-string keys for maps, c4::csubstr is used for convenience.
    /// Crucially. map_access currently expects a reference to the key returned.
    /// This is not possible, since there is no space to store the reference.
    /// So, the actual csubstr is returned, even though it means having to register
    /// more string accesses.
    ///
    /// Also, yaml_wrapper_data cannot be constructed from elementary values,
    /// at least yet - it would be impossible to index the map
    /// if objects of key_type could not be created.)
    ///
    /// Also note that rapidyaml has a separation between scalars, sequences,
    /// and mappings. The key of a pair is part of the pair, so it is not possible
    /// to obtain a yaml_wrapper_data referring to a key (and neither is it possible
    /// to obtain one pointing to a value - a yaml_wrapper_data is taken to mean
    /// the value, since that's the only way that works, without adding extra fields
    /// or tag bits to yaml_wrapper_data.
    struct J_HIDDEN yaml_map_access {
      using key_type J_NO_DEBUG_TYPE = c4::csubstr;
      using value_type J_NO_DEBUG_TYPE = yaml_wrapper_data;

      /// \todo This is here only because map_access_registration insists on filling it.
      J_NO_DEBUG inline static const a::typed_access_definition * m_key_access_definition = nullptr;
      /// \todo This is here only because map_access_registration insists on filling it.
      J_NO_DEBUG inline static const properties::wrappers::wrapper_definition * m_value_wrapper_definition = nullptr;

      J_ALWAYS_INLINE static sz_t size(const yaml_map & c) noexcept {
        return c.tree->num_children(c.index);
      }

      /// \todo This is here only because map_access_registration insists on filling it.
      J_ALWAYS_INLINE static sz_t empty(const yaml_map & c) noexcept {
        return size(c) == 0UL;
      }

      J_ALWAYS_INLINE static properties::wrapper at(yaml_map & c, const c4::csubstr & key) {
        sz_t child = c.tree->find_child(c.index, key);
        if (child == c4::yml::npos) {
          throw j::exceptions::out_of_range_exception()
            << j::exceptions::message("YAML map key was out of range.");
        }

        // An object that both contains a reference to the YAML tree and points to the
        // correct item just does not exist statically - so a new wrapper is allocated
        // on the heap.
        return properties::wrappers::wrapper{
          properties::detail::wrapper_take_ownership_tag,
          yaml_variant_wrapper_definition,
          (void*)::new yaml_wrapper_data{c.tree, child}};
      }


      // Iterator functions

      J_ALWAYS_INLINE static const c4::csubstr & get_iterator_key(a::iterator_data & it) {
        yaml_wrapper_data & data = it.as<yaml_wrapper_data>();
        J_ASSERT(data.tree->has_key(data.index), "YAML map item does not have a key.");
        return data.tree->key(data.index);
      }

      J_ALWAYS_INLINE static yaml_wrapper_data & get_iterator_value(a::iterator_data & it) {
        yaml_wrapper_data & data = it.as<yaml_wrapper_data>();
        // Check for *key*. not val - key is required for a map entry, value may be empty.
        J_ASSERT(data.tree->has_key(data.index), "YAML map item does not have a key.");
        return data;
      }

      J_ALWAYS_INLINE static a::iterator_data begin(yaml_map & c) {
        return a::iterator_data{yaml_wrapper_data{c.tree, c.tree->first_child(c.index)}};
      }

      J_ALWAYS_INLINE static a::iterator_data end(yaml_map & c) {
        return a::iterator_data{yaml_wrapper_data{c.tree, c4::yml::NONE}};
      }

      J_ALWAYS_INLINE static a::iterator_data find(yaml_map & c, const c4::csubstr & key) {
        return a::iterator_data{yaml_wrapper_data{c.tree, c.tree->find_child(c.index, c4::csubstr(key.data(), key.size()))}};
      }

      J_ALWAYS_INLINE static void advance_iterator(a::iterator_data & it) {
        yaml_wrapper_data & data = it.as<yaml_wrapper_data>();
        data.index = data.tree->next_sibling(data.index);
      }

      J_ALWAYS_INLINE static a::iterator_data copy_iterator(const a::iterator_data & src) {
        return src;
      }

      J_ALWAYS_INLINE static bool iterator_equals(const a::iterator_data & lhs, const a::iterator_data & rhs) noexcept {
        return lhs.as<yaml_wrapper_data>().index == rhs.as<yaml_wrapper_data>().index;
      }

      // Iterator needs no delete
      static constexpr null_t release_iterator = nullptr;

      // Unimplemented mutators

      J_ALWAYS_INLINE static void clear(yaml_map &) { throw_on_mutate(); }

      J_ALWAYS_INLINE static pair<a::iterator_data, bool> insert(
        yaml_map &, const c4::csubstr &, const yaml_wrapper_data &)
      { throw_on_mutate(); }

      J_ALWAYS_INLINE static a::iterator_data replace(
        yaml_map &,
        const c4::csubstr &,
        const yaml_wrapper_data &)
      { throw_on_mutate(); }

      J_ALWAYS_INLINE static sz_t erase_by_key(yaml_map &, const c4::csubstr &)
      { throw_on_mutate(); }

      J_ALWAYS_INLINE static a::iterator_data erase_by_iterator(yaml_map &, const a::iterator_data &)
      { throw_on_mutate(); }
    };

    using yaml_map_registration_t J_NO_DEBUG_TYPE = properties::map_access_registration<
      yaml_map,             // "Map", i.e. a yaml_wrapper_data by other name
      yaml_map_access,      // Wrapper class
      c4::csubstr,          // Key type
      yaml_wrapper_data     // Value (mapped) type
    >;

    const yaml_map_registration_t map_reg("yaml::map");



    struct J_HIDDEN yaml_list_access {
      J_NO_DEBUG inline static const a::list_access_definition * m_list_definition = nullptr;

      J_ALWAYS_INLINE static sz_t get_size(const yaml_list & c) noexcept {
        return c.tree->num_children(c.index);
      }

      static bool empty(const yaml_list & c) noexcept {
        return get_size(c) == 0;
      }

      J_ALWAYS_INLINE static properties::wrapper at(yaml_list & c, sz_t index) {
        sz_t child = c.tree->child(c.index, index);
        if (child == c4::yml::npos) {
          throw j::exceptions::out_of_range_exception()
            << j::exceptions::message("YAML list index was out of range.");
        }

        // An object that both contains a reference to the YAML tree and points to the
        // correct item just does not exist statically - so a new wrapper is allocated
        // on the heap.
        return properties::wrappers::wrapper{
          properties::detail::wrapper_take_ownership_tag,
          yaml_variant_wrapper_definition,
          (void*)::new yaml_wrapper_data{c.tree, child}};
      }



      // Iterator functions
      static void advance_iterator(a::iterator_data & it) noexcept {
        auto & d = it.as<yaml_wrapper_data>();
        d.index = d.tree->next_sibling(d.index);
      }

      static bool iterator_equals(const a::iterator_data & lhs, const a::iterator_data & rhs) noexcept {
        return lhs.as<const yaml_wrapper_data>().index == rhs.as<const yaml_wrapper_data>().index;
      }

      static properties::wrappers::wrapper get_iterator_value(const a::iterator_data & it) noexcept {
        return properties::wrappers::wrapper(
          properties::detail::wrapper_take_ownership_tag,
          yaml_variant_wrapper_definition,
          (void*)::new yaml_wrapper_data(it.as<const yaml_wrapper_data>()));
      }

      static a::list_iterator begin(yaml_list & c) {
        return a::list_iterator(yaml_list_access_definition,
                                a::iterator_data(
                                  yaml_wrapper_data{
                                    c.tree,
                                    c.tree->first_child(c.index)
                                  }));
      }

      static a::list_iterator end(yaml_list & c) {
        return a::list_iterator(yaml_list_access_definition,
                                a::iterator_data(
                                  yaml_wrapper_data{c.tree, c4::yml::npos}));
      }


      // Unimplemented mutators

      static void push_back(yaml_list &, const yaml_wrapper_data &) { throw_on_mutate(); }

      static a::list_iterator insert(yaml_list &, const a::list_iterator &, const yaml_wrapper_data &)
      { throw_on_mutate(); }

      static void erase_by_index(yaml_list &, sz_t) { throw_on_mutate(); }

      static a::list_iterator erase_by_iterator(yaml_list &, const a::list_iterator &) { throw_on_mutate(); }

      static void clear(yaml_list &) { throw_on_mutate(); }

      J_ALWAYS_INLINE J_HIDDEN J_NO_DEBUG
      static void initialize(const properties::wrappers::wrapper_definition *) noexcept {
      }
    };

    using yaml_list_registration_t J_NO_DEBUG_TYPE = properties::list_access_registration<
      yaml_list,             // "Map", i.e. a yaml_wrapper_data by other name
      yaml_list_access,      // Wrapper class
      yaml_wrapper_data      // Value (mapped) type
    >;

    const yaml_list_registration_t list_reg("yaml::list");



    inline properties::PROPERTY_TYPE check_exponent(const c4::csubstr & val) noexcept {
      if (val.size() < 2 || val[0] != 'e' || val[0] != 'E') {
        return properties::PROPERTY_TYPE::STRING;
      }
      sz_t i = 1;
      if (val[1] == '+' || val[2] == '-') {
        if (J_UNLIKELY(val.size() < 3)) {
          return properties::PROPERTY_TYPE::STRING;
        }
        i = 2;
      }
      // YAML allows (potentially repeating) leading zeroes in the exponent.
      for (; i < val.size(); ++i) {
        if (J_UNLIKELY(!is_digit(val[i]))) {
          return properties::PROPERTY_TYPE::STRING;
        }
      }
      return properties::PROPERTY_TYPE::FLOAT;
    }

    inline properties::PROPERTY_TYPE check_float(const c4::csubstr & val) noexcept {
      if (val[0] != '.') {
        // YAML allows neither repeating zeroes nor a missing zero before a decimal separator.
        // However, zero with an exponent is allowed.
        return check_exponent(val.sub(1));
      }

      // YAML allows nothing following the decimal separator (or exponent directly following)
      for (sz_t i = 1; i < val.size(); ++i) {
        if (!is_digit(val[i])) {
          return check_exponent(val.sub(i));
        }
      }
      return properties::PROPERTY_TYPE::FLOAT;
    }

    inline properties::PROPERTY_TYPE get_number_type(const c4::csubstr & val) noexcept {
      if (val[0] == '0') {
        if (val.size() == 1) {
          return properties::PROPERTY_TYPE::INT;
        }
        return check_float(val.sub(1));
      } else {
        for (sz_t i = 1; i < val.size(); ++i) {
          if (!is_digit(val[i])) {
            return check_float(val.sub(i));
          }
        }
        return properties::PROPERTY_TYPE::INT;
      }
    }

    inline properties::PROPERTY_TYPE get_scalar_type(const c4::csubstr & val) noexcept {
      if (val.empty()) {
        return properties::PROPERTY_TYPE::STRING;
      }

      switch (val[0]) {
      case 'n':
        if (val == "null") {
          return properties::PROPERTY_TYPE::NIL;
        }
        break;
      case 'f':
        if (val == "false") {
          return properties::PROPERTY_TYPE::BOOL;
        }
        break;
      case 't':
        if (val == "true") {
          return properties::PROPERTY_TYPE::BOOL;
        }
        break;
      case '-':
        if (J_UNLIKELY(val.size() == 1)) {
          break;
        }
        return get_number_type(val.sub(1));
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        return get_number_type(val);
      default:
        break;
      }
      return properties::PROPERTY_TYPE::STRING;
    }

    J_INLINE_GETTER properties::PROPERTY_TYPE get_yaml_variant_type(const void * variant_) noexcept {
      auto variant = reinterpret_cast<const yaml_wrapper_data*>(variant_);
      u32_t type = variant->tree->type(variant->index);
      bool is_key_quoted = type & c4::yml::NodeType_e::KEYQUO,
           is_val_quoted = type & c4::yml::NodeType_e::VALQUO;
      type &= ~c4::yml::NodeType_e::KEYQUO & ~c4::yml::NodeType_e::VALQUO;

      switch (type) {
      case c4::yml::NodeType_e::VAL: case c4::yml::NodeType_e::KEYVAL:
        // TODO How to know if it was quoted before rapidyaml parsed it?
        return get_scalar_type(variant->tree->val(variant->index));

      case c4::yml::NodeType_e::MAP: case c4::yml::NodeType_e::KEYMAP:
        return properties::PROPERTY_TYPE::MAP;

      case c4::yml::NodeType_e::SEQ: case c4::yml::NodeType_e::KEYSEQ:
        return properties::PROPERTY_TYPE::LIST;

      case c4::yml::NodeType_e::KEY:
        // Only key, actually empty.
        return properties::PROPERTY_TYPE::EMPTY;
      case c4::yml::NodeType_e::NOTYPE:
      case c4::yml::NodeType_e::KEYREF: case c4::yml::NodeType_e::VALREF:
      case c4::yml::NodeType_e::KEYANCH: case c4::yml::NodeType_e::VALANCH:
      case c4::yml::NodeType_e::KEYTAG: case c4::yml::NodeType_e::VALTAG:
      case c4::yml::NodeType_e::DOCMAP: case c4::yml::NodeType_e::DOCSEQ: case c4::yml::NodeType_e::DOCVAL:
      case c4::yml::NodeType_e::STREAM: case c4::yml::NodeType_e::DOC:
        // TODO: Anchors, references, tags, and documents are not supported
        return properties::PROPERTY_TYPE::EMPTY;

      default:
        J_UNREACHABLE();
      }
    }

    properties::access::typed_access get_yaml_variant_value(void * variant_) {
      auto variant = reinterpret_cast<yaml_wrapper_data*>(variant_);
      u32_t type = variant->tree->type(variant->index);
      // variant->tree->type(variant->index);
      bool is_key_quoted = type & c4::yml::NodeType_e::KEYQUO,
           is_val_quoted = type & c4::yml::NodeType_e::VALQUO;
      type &= ~c4::yml::NodeType_e::KEYQUO & ~c4::yml::NodeType_e::VALQUO;
      switch (type) {
      case c4::yml::NodeType_e::VAL: case c4::yml::NodeType_e::KEYVAL: {
        const c4::csubstr & str = variant->tree->val(variant->index);
        // TODO: Identify if the string represents a float or int.
        // ?? How to know if it was quoted before rapidyaml parsed it?
        switch (get_scalar_type(str)) {
        case properties::PROPERTY_TYPE::NIL:
          return properties::access::typed_access(
            p::detail::access_in_place_tag,
            *a::typed_access_definition_v<null_t>,
            (void*)nullptr);
        case properties::PROPERTY_TYPE::INT:
          return properties::access::typed_access(
            p::detail::access_in_place_tag,
            *a::typed_access_definition_v<i64_t>,
            reinterpret_cast<void*>(strings::parse_decimal_integer<i64_t>(str.data(), str.size())));
        case properties::PROPERTY_TYPE::FLOAT: {
          union {
            void * v;
            double d;
          } reinterpret;
          reinterpret.d = strings::parse_float<double>(str.data(), str.size());
          return properties::access::typed_access(
            p::detail::access_in_place_tag,
            *a::typed_access_definition_v<double>,
            reinterpret.v);
        }
        case properties::PROPERTY_TYPE::BOOL:
          return properties::access::typed_access(
            p::detail::access_in_place_tag,
            *a::typed_access_definition_v<bool>,
            reinterpret_cast<void*>(str == "true"));
        case properties::PROPERTY_TYPE::STRING:
          return properties::access::typed_access(
            *csubstr_access_definition,
            static_cast<void*>(&const_cast<c4::csubstr&>(str)));
        default:
          J_UNREACHABLE();
        }
      }
      case c4::yml::NodeType_e::MAP: case c4::yml::NodeType_e::KEYMAP:
        return properties::access::typed_access(yaml_map_registration_t::definition, variant);
      case c4::yml::NodeType_e::SEQ: case c4::yml::NodeType_e::KEYSEQ:
        return properties::access::typed_access(yaml_list_registration_t::definition, variant);
      default:
        J_THROW("TODO: Unexcepted YAML variant value type.");
      }
    }

    void set_yaml_variant_value(void *, const properties::typed_access &) {
      throw_on_mutate();
    }

    properties::wrapper create_yaml_variant_wrapper(const properties::wrapper_definition & d, properties::typed_access & a) {
      if (&a.definition() == &yaml_map_registration_t::definition) {
        return properties::wrapper{
          properties::detail::wrapper_take_ownership_tag,
          d,
          new yaml_wrapper_data(*reinterpret_cast<yaml_wrapper_data*>(a.as_void_star()))
        };
      }
      J_THROW("Unexpected YAML variant wrapper type.");
    }
  }

  const properties::wrapper_definition yaml_variant_wrapper_definition{
    properties::detail::make_value_definition<yaml_wrapper_data>(),
    get_yaml_variant_type,
    set_yaml_variant_value,
    get_yaml_variant_value,
    create_yaml_variant_wrapper,
    trivial_array<const properties::access::typed_access_definition *>{
      containers::move,
      {
        csubstr_access_definition,
        properties::access::typed_access_definition_v<i64_t>,
        properties::access::typed_access_definition_v<bool>,
        properties::access::typed_access_definition_v<double>,
        &yaml_map_registration_t::definition
      }}};

  const properties::access::list_access_definition * yaml_list_access_definition = &yaml_list_registration_t::definition;

  namespace {
    J_NO_DEBUG properties::wrapper_registration<yaml_wrapper_data> reg{&yaml_variant_wrapper_definition};
  }
}
