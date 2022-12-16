#pragma once

#include "containers/hash_map_fwd.hpp"
#include "strings/string.hpp"
#include "rendering/vulkan/spirv/instruction.hpp"
#include "rendering/data_types/data_type.hpp"

namespace j::rendering::vulkan::inline spirv {
  struct insn_type_struct;

  enum class type_kind : u8_t {
    none,

    void_type,
    boolean_type,
    integer_type,
    float_type,
    vector_type,
    matrix_type,
    image_type,
    sampled_image_type,
    array_type,
    runtime_array_type,
    pointer_type,
    struct_type,
  };

  constexpr u8_t type_info_bits(type_kind k) noexcept {
    switch (k) {
    case type_kind::none:
      J_THROW("Type cannot be none.");
    case type_kind::void_type:
    case type_kind::boolean_type:
    case type_kind::integer_type:
    case type_kind::float_type:
    case type_kind::vector_type:
    case type_kind::matrix_type:
      return 12U;
    case type_kind::image_type:
    case type_kind::sampled_image_type:
      return 19U;
    case type_kind::array_type:
    case type_kind::runtime_array_type:
      return 4U;
    case type_kind::pointer_type:
      return 8U;
    case type_kind::struct_type:
      return 52U;
    }
  }

  enum class image_depth_type : u8_t {
    not_depth = 0U,
    depth = 1U,
    unknown = 2U,
  };

  enum class image_sampling_type : u8_t {
    unknown = 0U,
    sampled = 1U,
    storage = 2U,
  };

  enum class image_dimensionality : u8_t {
    dim_1d = 0U,
    dim_2d,
    dim_3d,
    dim_cube,
    dim_buffer,
    dim_subpass_data,
    unknown,
  };

  class type_table;

  class void_type_info;
  class boolean_type_info;
  class integer_type_info;
  class float_type_info;
  class vector_type_info;
  class matrix_type_info;
  class image_type_info;
  class sampled_image_type_info;
  class array_type_info;
  class runtime_array_type_info;
  class pointer_type_info;
  class struct_type_info;

  class type_info {
  private:
    type_info(type_kind kind, data_types::data_type t) noexcept;

    type_info(uptr_t value) noexcept
      : m_value(value)
    {
      if (is_heap_allocated()) {
        reference();
      }
    }
  public:
    type_info() noexcept = default;

    type_info(const type_table & table, const instruction & insn) noexcept;

    type_info(const type_info & rhs) noexcept
      : type_info(rhs.m_value)
    { }

    type_info & operator=(const type_info & rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (is_heap_allocated()) {
          release();
        }
        m_value = rhs.m_value;
        if (is_heap_allocated()) {
          reference();
        }
      }
      return *this;
    }

    type_info(type_info && rhs) noexcept
      : m_value(rhs.m_value)
    {
      rhs.m_value = 0;
    }

    void_type_info & as_void() noexcept;
    const void_type_info & as_void() const noexcept {
      return const_cast<type_info*>(this)->as_void();
    }
    boolean_type_info & as_boolean() noexcept;
    const boolean_type_info & as_boolean() const noexcept {
      return const_cast<type_info*>(this)->as_boolean();
    }
    integer_type_info & as_integer() noexcept;
    const integer_type_info & as_integer() const noexcept {
      return const_cast<type_info*>(this)->as_integer();
    }
    float_type_info & as_float() noexcept;
    const float_type_info & as_float() const noexcept {
      return const_cast<type_info*>(this)->as_float();
    }
    vector_type_info & as_vector() noexcept;
    const vector_type_info & as_vector() const noexcept {
      return const_cast<type_info*>(this)->as_vector();
    }
    matrix_type_info & as_matrix() noexcept;
    const matrix_type_info & as_matrix() const noexcept {
      return const_cast<type_info*>(this)->as_matrix();
    }
    image_type_info & as_image() noexcept;
    const image_type_info & as_image() const noexcept {
      return const_cast<type_info*>(this)->as_image();
    }
    sampled_image_type_info & as_sampled_image() noexcept;
    const sampled_image_type_info & as_sampled_image() const noexcept {
      return const_cast<type_info*>(this)->as_sampled_image();
    }
    array_type_info & as_array() noexcept;
    const array_type_info & as_array() const noexcept {
      return const_cast<type_info*>(this)->as_array();
    }
    runtime_array_type_info & as_runtime_array() noexcept;
    const runtime_array_type_info & as_runtime_array() const noexcept {
      return const_cast<type_info*>(this)->as_runtime_array();
    }
    pointer_type_info & as_pointer() noexcept;
    const pointer_type_info & as_pointer() const noexcept {
      return const_cast<type_info*>(this)->as_pointer();
    }
    struct_type_info & as_struct() noexcept;
    const struct_type_info & as_struct() const noexcept {
      return const_cast<type_info*>(this)->as_struct();
    }

    type_info & operator=(type_info && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (is_heap_allocated()) {
          release();
        }
        m_value = rhs.m_value;
        rhs.m_value = 0;
      }
      return *this;
    }

    explicit operator bool() const noexcept
    { return m_value; }

    bool operator!() const noexcept
    { return !m_value; }

    type_kind kind() const noexcept {
      return static_cast<type_kind>(m_value & 0xF);
    }

    template<typename Fn>
    decltype(auto) visit(Fn && fn) const;

    ~type_info() {
      if (is_heap_allocated()) {
        release();
      }
    }
  protected:
    type_info inner() const noexcept {
      return type_info(m_value >> type_info_bits(kind()));
    }

    type_info wrapped_as(type_kind wrapper, uptr_t metadata) const noexcept {
      return type_info((m_value << type_info_bits(wrapper)) | static_cast<uptr_t>(wrapper) | (static_cast<uptr_t>(metadata) << 4ULL));
    }

    uptr_t value() const noexcept {
      return (m_value >> 4ULL) & ((1ULL << (type_info_bits(kind()) - 4U)) - 1ULL);
    }

    template<typename T>
    T value_as() const noexcept {
      return (T)value();
    }

    bool is_heap_allocated() const noexcept {
      for (uptr_t v = m_value; v; v = v >> type_info_bits(static_cast<type_kind>(v & 0x0F))) {
        if (static_cast<type_kind>(v & 0x0F) == type_kind::struct_type) {
          return true;
        }
      }
      return false;
    }

    void release() noexcept;
    void reference() noexcept;

    uptr_t m_value = 0;
  };

  class basic_type_info_base : public type_info {
  public:
    data_types::data_type data_type() const noexcept {
      return value_as<data_types::data_type>();
    }
  };

  class void_type_info final : public basic_type_info_base { };

  class boolean_type_info final : public basic_type_info_base { };

  class integer_type_info final : public basic_type_info_base { };

  class float_type_info final : public basic_type_info_base { };

  class vector_type_info final : public basic_type_info_base { };

  class matrix_type_info final : public basic_type_info_base { };

  class image_type_info_base : public type_info {
  public:
    image_depth_type depth_type() const noexcept {
      return (image_depth_type)(value() & 3);
    }

    image_sampling_type sampling_type() const noexcept {
      return (image_sampling_type)((value() >> 2) & 3);
    }

    image_dimensionality dimensionality() const noexcept {
      return (image_dimensionality)((value() >> 4) & 7);
    }

    bool is_array() const noexcept {
      return value() & (1 << 7);
    }

    bool is_multisampled() const noexcept {
      return value() & (1 << 8);
    }

    image_format format() const noexcept {
      return (image_format)(value() >> 9);
    }
  };

  class image_type_info final : public image_type_info_base { };

  class sampled_image_type_info final : public image_type_info_base { };

  class array_type_info_base : public type_info {
  public:
    type_info element_type() const noexcept {
      return inner();
    }
  };

  class array_type_info final : public array_type_info_base { };
  class runtime_array_type_info final : public array_type_info_base { };

  class pointer_type_info final : public type_info {
  public:
    type_info target_type() const noexcept {
      return inner();
    }

    enum storage_class storage_class() const noexcept {
      return value_as<enum storage_class>();
    }
  };

  class struct_member_info final {
  public:
    struct_member_info() noexcept = default;
    struct_member_info(const type_info & type) noexcept
      : m_type(type)
    {
    }

    void set_type(const type_info & type) noexcept {
      m_type = type;
    }

    void set_name(const char * name) noexcept {
      m_name = name;
    }

    void set_offset(u32_t offset) noexcept {
      m_offset = offset;
    }

    const type_info & type() const noexcept {
      return m_type;
    }

    const strings::string & name() const noexcept {
      return m_name;
    }

    u32_t offset() const noexcept {
      return m_offset;
    }
  private:
    type_info m_type;
    strings::string m_name;
    u32_t m_offset = 0U;
  };

  struct struct_info final {
    static struct_info * allocate(const type_table & table, const insn_type_struct & insn);
    void add_reference();
    void remove_reference();

    u32_t size;
    u32_t reference_count = 1U;
    strings::string name;
    struct_member_info members[1];
  };

  class struct_type_info final : public type_info {
  public:
    struct_member_info * begin() noexcept {
      return value_as<struct_info *>()->members;
    }
    const struct_member_info * begin() const noexcept {
      return value_as<const struct_info *>()->members;
    }

    u32_t size() const noexcept {
      return value_as<const struct_info *>()->size;
    }

    const struct_member_info * end() const noexcept {
      return begin() + size();
    }
    struct_member_info * end() noexcept {
      return begin() + size();
    }

    const strings::string & name() const noexcept {
      return value_as<const struct_info *>()->name;
    }

    void set_name(const char * name) const noexcept {
      value_as<struct_info *>()->name = name;
    }

    struct_member_info & operator[](u32_t i) noexcept;
    const struct_member_info & operator[](u32_t i) const noexcept;
  };

  template<typename Fn>
  decltype(auto) type_info::visit(Fn && fn) const {
    switch (kind()) {
    case type_kind::void_type:
      return static_cast<Fn &&>(fn)(static_cast<const void_type_info &>(*this));
    case type_kind::boolean_type:
      return static_cast<Fn &&>(fn)(static_cast<const boolean_type_info &>(*this));
    case type_kind::integer_type:
      return static_cast<Fn &&>(fn)(static_cast<const integer_type_info &>(*this));
    case type_kind::float_type:
      return static_cast<Fn &&>(fn)(static_cast<const float_type_info &>(*this));
    case type_kind::vector_type:
      return static_cast<Fn &&>(fn)(static_cast<const vector_type_info &>(*this));
    case type_kind::matrix_type:
      return static_cast<Fn &&>(fn)(static_cast<const matrix_type_info &>(*this));
    case type_kind::image_type:
      return static_cast<Fn &&>(fn)(static_cast<const image_type_info &>(*this));
    case type_kind::sampled_image_type:
      return static_cast<Fn &&>(fn)(static_cast<const sampled_image_type_info &>(*this));
    case type_kind::array_type:
      return static_cast<Fn &&>(fn)(static_cast<const array_type_info &>(*this));
    case type_kind::runtime_array_type:
      return static_cast<Fn &&>(fn)(static_cast<const runtime_array_type_info &>(*this));
    case type_kind::struct_type:
      return static_cast<Fn &&>(fn)(static_cast<const struct_type_info &>(*this));
    case type_kind::pointer_type:
      return static_cast<Fn &&>(fn)(static_cast<const pointer_type_info &>(*this));
    case type_kind::none:
      J_UNREACHABLE();
    }
  }

  inline void_type_info & type_info::as_void() noexcept {
    J_ASSERT(kind() == type_kind::void_type, "Invalid type");
    return static_cast<void_type_info &>(*this);
  }
  inline boolean_type_info & type_info::as_boolean() noexcept {
    J_ASSERT(kind() == type_kind::boolean_type, "Invalid type");
    return static_cast<boolean_type_info &>(*this);
  }
  inline integer_type_info & type_info::as_integer() noexcept {
    J_ASSERT(kind() == type_kind::integer_type, "Invalid type");
    return static_cast<integer_type_info &>(*this);
  }
  inline float_type_info & type_info::as_float() noexcept {
    J_ASSERT(kind() == type_kind::float_type, "Invalid type");
    return static_cast<float_type_info &>(*this);
  }
  inline vector_type_info & type_info::as_vector() noexcept {
    J_ASSERT(kind() == type_kind::vector_type, "Invalid type");
    return static_cast<vector_type_info &>(*this);
  }
  inline matrix_type_info & type_info::as_matrix() noexcept {
    J_ASSERT(kind() == type_kind::matrix_type, "Invalid type");
    return static_cast<matrix_type_info &>(*this);
  }
  inline image_type_info & type_info::as_image() noexcept {
    J_ASSERT(kind() == type_kind::image_type, "Invalid type");
    return static_cast<image_type_info &>(*this);
  }
  inline sampled_image_type_info & type_info::as_sampled_image() noexcept {
    J_ASSERT(kind() == type_kind::sampled_image_type, "Invalid type");
    return static_cast<sampled_image_type_info &>(*this);
  }
  inline array_type_info & type_info::as_array() noexcept {
    J_ASSERT(kind() == type_kind::array_type, "Invalid type");
    return static_cast<array_type_info &>(*this);
  }
  inline runtime_array_type_info & type_info::as_runtime_array() noexcept {
    J_ASSERT(kind() == type_kind::runtime_array_type, "Invalid type");
    return static_cast<runtime_array_type_info &>(*this);
  }
  inline pointer_type_info & type_info::as_pointer() noexcept {
    J_ASSERT(kind() == type_kind::pointer_type, "Invalid type");
    return static_cast<pointer_type_info &>(*this);
  }
  inline struct_type_info & type_info::as_struct() noexcept {
    J_ASSERT(kind() == type_kind::struct_type, "Invalid type");
    return static_cast<struct_type_info &>(*this);
  }
}

J_DECLARE_EXTERN_HASH_MAP(u32_t, j::rendering::vulkan::spirv::type_info);

namespace j::rendering::vulkan::inline spirv {
  class type_table final {
    using map_t = hash_map<u32_t, type_info>;
  public:
    J_INLINE_GETTER u32_t size() const
    { return m_map.size(); }

    J_INLINE_GETTER auto begin() const
    { return m_map.begin(); }

    J_INLINE_GETTER auto end() const
    { return m_map.end(); }

    void define_type(u32_t id, const instruction & insn);

    // J_INLINE_GETTER type_info & operator[](u32_t i)
    // { return m_map.at(i); }

    J_INLINE_GETTER type_info & at(u32_t i)
    { return m_map.at(i); }

    J_INLINE_GETTER const type_info & at(u32_t i) const
    { return m_map.at(i); }

    J_INLINE_GETTER bool contains(u32_t i) const noexcept
    { return m_map.contains(i); }
  private:
    map_t m_map;
  };
}
