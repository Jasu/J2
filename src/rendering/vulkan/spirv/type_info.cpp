#include "rendering/vulkan/spirv/type_info.hpp"
#include "containers/hash_map.hpp"
#include "exceptions/assert_lite.hpp"

#include <rendering/vulkan/spirv/instructions.hpp>

J_DEFINE_EXTERN_HASH_MAP(u32_t, j::rendering::vulkan::spirv::type_info);

namespace j::rendering::vulkan::inline spirv {
  type_info::type_info(type_kind kind, data_types::data_type t) noexcept
    : m_value(static_cast<uptr_t>(kind) | (static_cast<uptr_t>(t) << 4ULL))
  { }

  type_info::type_info(const type_table & table, const instruction & insn) noexcept {
    switch (insn.opcode()) {
    case opcode::op_type_void:
      m_value = static_cast<uptr_t>(type_kind::void_type);
      break;
    case opcode::op_type_bool:
      m_value = static_cast<uptr_t>(type_kind::boolean_type) | (static_cast<uptr_t>(data_types::data_type::boolean) << 4);
      break;
    case opcode::op_type_int: {
      auto & i = reinterpret_cast<const insn_type_int &>(insn);
      auto w = i.width;
      if (i.signedness) {
        if (w == 8) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::s8) << 4);
        } else if (w == 16) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::s16) << 4);
        } else if (w == 32) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::s32) << 4);
        } else if (w == 64) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::s64) << 4);
        } else {
          J_THROW("Unsupported integer width");
        }
      } else {
        if (w == 8) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::u8) << 4);
        } else if (w == 16) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::u16) << 4);
        } else if (w == 32) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::u32) << 4);
        } else if (w == 64) {
          m_value = static_cast<uptr_t>(type_kind::integer_type) | (static_cast<uptr_t>(data_types::data_type::u64) << 4);
        } else {
          J_THROW("Unsupported integer width");
        }
      }
      break;
    }
    case opcode::op_type_float: {
      auto & i = reinterpret_cast<const insn_type_float &>(insn);
      auto w = i.width;
      if (w == 16) {
        m_value = static_cast<uptr_t>(type_kind::float_type) | (static_cast<uptr_t>(data_types::data_type::fp16) << 4);
      } else if (w == 32) {
        m_value = static_cast<uptr_t>(type_kind::float_type) | (static_cast<uptr_t>(data_types::data_type::fp32) << 4);
      } else if (w == 64) {
        m_value = static_cast<uptr_t>(type_kind::float_type) | (static_cast<uptr_t>(data_types::data_type::fp64) << 4);
      } else {
        J_THROW("Unsupported float width");
      }
      break;
    }
    case opcode::op_type_vector: {
      auto & i = reinterpret_cast<const insn_type_vector &>(insn);
      auto & element = table.at(i.component_type);
      m_value = (uptr_t)type_kind::vector_type | ((uptr_t)data_types::with_cols(static_cast<const basic_type_info_base &>(element).data_type(), i.component_count) << 4);
      break;
    }
    case opcode::op_type_matrix: {
      auto & i = reinterpret_cast<const insn_type_matrix &>(insn);
      auto & column = table.at(i.column_type);
      m_value = (uptr_t)type_kind::matrix_type | ((uptr_t)data_types::with_rows(static_cast<const basic_type_info_base &>(column).data_type(), i.column_count) << 4);
      break;
    }
    case opcode::op_type_image: {
      auto & i = reinterpret_cast<const insn_type_image &>(insn);
      m_value = (uptr_t)type_kind::image_type
        | ((uptr_t)i.depth << 4)
        | (uptr_t)(i.sampled << 6)
        | (uptr_t)(i.dim << 8)
        | (uptr_t)(i.arrayed << 11)
        | (uptr_t)(i.ms << 12)
        | ((uptr_t)i.image_format << 13);
      break;
    }
    case opcode::op_type_sampled_image:
      m_value |= (uptr_t)type_kind::sampled_image_type
        | (table.at(reinterpret_cast<const insn_type_sampled_image &>(insn).image_type).m_value & ~0xFULL);
      break;
    case opcode::op_type_array: {
      auto & i = reinterpret_cast<const insn_type_array &>(insn);
      *this = table.at(i.element_type).wrapped_as(type_kind::array_type, 0);
      break;
    }
    case opcode::op_type_runtime_array: {
      auto & i = reinterpret_cast<const insn_type_runtime_array &>(insn);
      *this = table.at(i.element_type).wrapped_as(type_kind::runtime_array_type, 0);
      break;
    }
    case opcode::op_type_struct: {
      auto & i = reinterpret_cast<const insn_type_struct &>(insn);
      struct_info * info = struct_info::allocate(table, i);
      m_value = (uptr_t)info << 4ULL | static_cast<uptr_t>(type_kind::struct_type);
      break;
    }
    case opcode::op_type_pointer: {
      auto & i = reinterpret_cast<const insn_type_pointer &>(insn);
      *this = table.at(i.type).wrapped_as(type_kind::pointer_type, (u8_t)i.storage_class);
      break;
    }
    default:
      J_FAIL("Unsupported opcode for type definition");
    }
  }

  void type_info::release() noexcept {
    for (uptr_t v = m_value; v; v = v >> type_info_bits(static_cast<type_kind>(v & 0x0F))) {
      if (static_cast<type_kind>(v & 0x0F) == type_kind::struct_type) {
        reinterpret_cast<struct_info*>(v >> 4ULL)->remove_reference();
        return;
      }
    }
  }

  void type_info::reference() noexcept {
    for (uptr_t v = m_value; v; v = v >> type_info_bits(static_cast<type_kind>(v & 0x0F))) {
      if (static_cast<type_kind>(v & 0x0F) == type_kind::struct_type) {
        reinterpret_cast<struct_info*>(v >> 4ULL)->add_reference();
        return;
      }
    }
  }

  struct_info * struct_info::allocate(const type_table & table, const insn_type_struct & insn) {
    const u32_t sz = insn.member_types().size();
    struct_info * result = ::new (::j::allocate((sz - 1U) * sizeof(struct_member_info) + sizeof(struct_info))) struct_info;
    result->size = sz;
    for (u32_t i = 0; i < sz; ++i) {
      ::new (&result->members[i]) struct_member_info(table.at(insn.member_types()[i]));
    }
    return result;
  }

  void struct_info::add_reference() {
    J_ASSERT_NOT_NULL(reference_count);
    ++reference_count;
  }

  void struct_info::remove_reference() {
    J_ASSERT_NOT_NULL(reference_count);
    if (!--reference_count) {
      for (u32_t i = 0; i < size; ++i) {
        members[i].~struct_member_info();
      }
    }
  }

  struct_member_info & struct_type_info::operator[](u32_t i) noexcept {
    J_ASSERT(i < size(), "Out of bounds");
    return begin()[i];
  }
  const struct_member_info & struct_type_info::operator[](u32_t i) const noexcept {
    J_ASSERT(i < size(), "Out of bounds");
    return begin()[i];
  }

  void type_table::define_type(u32_t id, const instruction & insn) {
    [[maybe_unused]] const bool did_insert = m_map.emplace(id, *this, insn).second;
    J_ASSERT(did_insert, "Duplicate type");
  }
}
