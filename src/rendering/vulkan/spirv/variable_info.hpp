#pragma once

#include "rendering/vulkan/spirv/type_info.hpp"
#include "strings/string_view.hpp"

namespace j::rendering::vulkan::inline spirv {
  struct insn_spec_constant;
  struct insn_variable;
  struct insn_decorate;

  enum class variable_kind : u8_t {
    /// Variables whose storage_classes are not specially handled map to this.
    other,
    /// Input to the shader.
    input,
    /// Output from the shader.
    output,
    /// Uniform variable.
    uniform,
    /// Storage class push constant.
    push_constant,
    /// Specialization constant.
    ///
    /// \note This does not map to a storage class and is not technically even a variable.
    spec_constant,
  };

  enum class interpolation : u8_t {
    smooth,
    flat,
    no_perspective,
  };

  /// Information about a uniform variable.
  class uniform_info final {
  public:
    J_ALWAYS_INLINE void reset() noexcept {
      m_descriptor_set = m_binding = U8_MAX;
    }

    void set_descriptor_set(u8_t set)  noexcept {
      J_ASSUME(m_descriptor_set == U8_MAX);
      m_descriptor_set = set;
    }

    void set_binding(u8_t binding)  noexcept {
      J_ASSUME(m_binding);
      m_binding = binding;
    }

    u8_t descriptor_set() const noexcept {
      J_ASSUME(m_descriptor_set != U8_MAX);
      return m_descriptor_set;
    }
    u8_t binding() const noexcept {
      J_ASSUME(m_descriptor_set != U8_MAX);
      return m_binding;
    }
  private:
    u8_t m_descriptor_set = U8_MAX;
    u8_t m_binding = U8_MAX;
  };

  class specialization_constant_info final {
  public:
    J_ALWAYS_INLINE void reset() noexcept {
      m_id = U8_MAX;
    }

    u8_t id() const noexcept {
      J_ASSERT(m_id != U8_MAX, "No id.");
      return m_id;
    }

    void set_id(u8_t id) noexcept {
      J_ASSERT(m_id == U8_MAX, "Id was set twice.");
      m_id = id;
    }

  private:
    u8_t m_id = U8_MAX;
  };

  /// Information about shader input or output variable.
  class input_output_info final {
  public:
    constexpr input_output_info() noexcept = default;
    void reset() noexcept {
      m_location = U8_MAX;
      m_component = 63U;
      m_interpolation = interpolation::smooth;
    }

    void set_location(u8_t location) noexcept {
      J_ASSERT(m_location == U8_MAX, "Location was set twice.");
      m_location = location;
    }

    void set_component(u8_t component) noexcept {
      J_ASSERT(m_component == 63U, "Component was set twice.");
      m_component = component;
    }

    void set_interpolation(enum interpolation interpolation) noexcept {
      J_ASSERT(m_interpolation == interpolation::smooth, "Interpolation was set twice.");
      m_interpolation = interpolation;
    }

    J_INLINE_GETTER bool has_location() const noexcept
    { return m_location != U8_MAX; }

    J_INLINE_GETTER bool has_component() const noexcept
    { return m_component != 63U; }

    J_INLINE_GETTER u8_t location() const noexcept {
      J_ASSERT(m_component != U8_MAX, "Location not set.");
      return m_location;
    }

    J_INLINE_GETTER u8_t component() const noexcept {
      J_ASSERT(m_component != 63U, "Component not set.");
      return m_component;
    }

    J_INLINE_GETTER interpolation interpolation() const noexcept {
      return m_interpolation;
    }
  private:
    u8_t m_location = U8_MAX;
    u8_t m_component:6 = 63U;
    enum interpolation m_interpolation:2 = interpolation::smooth;
  };

  /// Information about a variable or a specialization constant.
  class variable_info final {
  public:
    constexpr variable_info() noexcept = default;

    /// Construct a normal (non-spec-const) variable.
    variable_info(const type_table & types, const insn_variable & insn);

    /// Construct a specialization constant variable.
    variable_info(const type_table & types, const insn_spec_constant & insn);

    void set_name(const char * J_NOT_NULL name) noexcept
    { m_name = name; }

    void add_decoration(const insn_decorate & insn) noexcept;

    J_INLINE_GETTER variable_kind kind() const noexcept
    { return m_kind; }

    const input_output_info & input_info() const noexcept {
      J_ASSUME(m_kind == variable_kind::input);
      return m_info.io;
    }

    const input_output_info & output_info() const noexcept {
      J_ASSUME(m_kind == variable_kind::output);
      return m_info.io;
    }

    const class uniform_info & uniform_info() const noexcept {
      J_ASSUME(m_kind == variable_kind::uniform);
      return m_info.uniform;
    }

    const class specialization_constant_info & specialization_constant_info() const noexcept {
      J_ASSUME(m_kind == variable_kind::spec_constant);
      return m_info.spec;
    }

    const strings::string & name() const noexcept
    { return m_name; }

    const type_info & type() const noexcept
    { return m_type; }
  private:
    type_info m_type;
    /// The name of the variable.
    strings::string m_name;
    variable_kind m_kind = variable_kind::other;
    union {
      input_output_info io;
      class uniform_info uniform;
      class specialization_constant_info spec;
    } m_info = { .io = input_output_info{} };
  };
}

J_DECLARE_EXTERN_HASH_MAP(u32_t, j::rendering::vulkan::spirv::variable_info);

namespace j::rendering::vulkan::inline spirv {
  class variable_table final {
    using map_t = hash_map<u32_t, variable_info>;
  public:
    void define_variable(const type_table & types, const insn_variable & insn);

    void define_specialization_constant(const type_table & types, const insn_spec_constant & insn);

    J_INLINE_GETTER u32_t size() const
    { return m_map.size(); }

    J_INLINE_GETTER auto begin() const
    { return m_map.begin(); }

    J_INLINE_GETTER auto end() const
    { return m_map.end(); }

    J_INLINE_GETTER variable_info & operator[](u32_t i)
    { return m_map[i]; }

    J_INLINE_GETTER variable_info & at(u32_t i)
    { return m_map.at(i); }

    J_INLINE_GETTER const variable_info & at(u32_t i) const
    { return m_map.at(i); }

    J_INLINE_GETTER bool contains(u32_t i) const noexcept
    { return m_map.contains(i); }
  private:
    map_t m_map;
  };
}
