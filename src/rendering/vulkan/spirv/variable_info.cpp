#include "variable_info.hpp"

#include "exceptions/assert_lite.hpp"
#include "containers/hash_map.hpp"
#include "logging/global.hpp"
#include <rendering/vulkan/spirv/instructions.hpp>

J_DEFINE_EXTERN_HASH_MAP(u32_t, j::rendering::vulkan::spirv::variable_info);

namespace j::rendering::vulkan::inline spirv {
  namespace {
    inline variable_kind storage_class_to_kind(storage_class s) noexcept {
      switch (s) {
      case storage_class::input:
        return variable_kind::input;
      case storage_class::output:
        return variable_kind::output;
      case storage_class::uniform_constant:
      case storage_class::uniform:
        return variable_kind::uniform;
      case storage_class::push_constant:
        return variable_kind::push_constant;
      default:
        return variable_kind::other;
      }
    }

    inline void assert_io([[maybe_unused]] variable_kind kind) {
      J_ASSERT(kind == variable_kind::input || kind == variable_kind::output,
               "Not an input or output variable.");
    }
  }

  variable_info::variable_info(const type_table & types, const insn_variable & insn)
    : m_type(types.at(insn.id_result_type)),
      m_kind(storage_class_to_kind(insn.storage_class))
  {
    if (m_kind == variable_kind::uniform) {
      m_info.uniform.reset();
    }
  }

  variable_info::variable_info(const type_table & types, const insn_spec_constant & insn)
    : m_type(types.at(insn.id_result_type)),
      m_kind(variable_kind::spec_constant)
  {
    m_info.spec.reset();
  }

  void variable_info::add_decoration(const insn_decorate & insn) noexcept {
    switch (insn.decoration) {
    case decoration::no_perspective:
      assert_io(m_kind);
      m_info.io.set_interpolation(interpolation::no_perspective);
      break;
    case decoration::flat:
      assert_io(m_kind);
      m_info.io.set_interpolation(interpolation::flat);
      break;
    case decoration::location:
      assert_io(m_kind);
      m_info.io.set_location(insn.decoration_args()[0]);
      break;
    case decoration::component:
      assert_io(m_kind);
      m_info.io.set_component(insn.decoration_args()[0]);
      break;
    case decoration::descriptor_set:
      J_ASSERT(m_kind == variable_kind::uniform, "Not a uniform variable.");
      m_info.uniform.set_descriptor_set(insn.decoration_args()[0]);
      break;
    case decoration::binding:
      J_ASSERT(m_kind == variable_kind::uniform, "Not a uniform variable.");
      m_info.uniform.set_binding(insn.decoration_args()[0]);
      break;
    case decoration::spec_id:
      J_ASSERT(m_kind == variable_kind::spec_constant, "Not a specialization constant.");
      m_info.spec.set_id(insn.decoration_args()[0]);
      break;
    default:
      J_DEBUG("Unprocessed decoration {}", insn.decoration);
      return;
    }
  }

  void variable_table::define_variable(const type_table & types,
                                       const insn_variable & insn)
  {
    const u32_t idx = insn.id_result;
    [[maybe_unused]] const bool did_insert = m_map.emplace(idx, types, insn).second;
    J_ASSERT(did_insert, "Duplicate variable");
  }

  void variable_table::define_specialization_constant(const type_table & types,
                                                      const insn_spec_constant & insn)
  {
    const u32_t idx = insn.id_result;
    [[maybe_unused]] const bool did_insert = m_map.emplace(idx, types, insn).second;
    J_ASSERT(did_insert, "Duplicate specialization constant");
  }
}
