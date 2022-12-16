#include "rendering/vulkan/spirv/shader_metadata.hpp"
#include "rendering/vulkan/spirv/header.hpp"
#include "rendering/vulkan/spirv/instruction_iterator.hpp"
#include "rendering/vulkan/spirv/specialization_constant_set.hpp"
#include "rendering/vulkan/spirv/visit.hpp"
#include "rendering/vertex_data/vertex_input_info.hpp"
#include <rendering/vulkan/spirv/instructions.hpp>

namespace j::rendering::vulkan::inline spirv {
  namespace {
    struct J_TYPE_HIDDEN instruction_visitor final {
      shader_metadata & result;

      template<typename T>
      void operator()(const T & insn) {
        result.types.define_type(insn.id_result, insn.instruction);
      }

      void operator()(const insn_variable & insn) {
        result.vars.define_variable(result.types, insn);
      }

      void operator()(const insn_spec_constant & insn) {
        result.vars.define_specialization_constant(result.types, insn);
      }
    };

    struct J_TYPE_HIDDEN annotation_visitor {
      shader_metadata & result;

      void operator()(const insn_name & insn) {
        const u32_t id = insn.target;
        if (result.types.contains(id)) {
          auto & t = result.types.at(id);
          if (t.kind() == type_kind::struct_type) {
            t.as_struct().set_name(insn.name());
          }
        } else if (result.vars.contains(id)) {
          result.vars.at(id).set_name(insn.name());
        }
      }

      void operator()(const insn_member_name & insn) {
        const u32_t id = insn.type;
        auto & t = result.types.at(id).as_struct();
        t[insn.member].set_name(insn.name());
      }

      void operator()(const insn_decorate & insn) {
        const u32_t id = insn.target;
        if (result.vars.contains(id)) {
          result.vars.at(id).add_decoration(insn);
        }
      }

      void operator()(const insn_member_decorate & insn) {
        const u32_t id = insn.structure_type;
        auto & t = result.types.at(id).as_struct();
        if (insn.decoration == decoration::offset) {
          t[insn.member].set_offset(insn.decoration_args()[0]);
        }
      }
    };

    struct J_TYPE_HIDDEN vertex_input_variable_visitor {
      data_types::data_type operator()(const integer_type_info & t) const {
        return t.data_type();
      }

      data_types::data_type operator()(const float_type_info & t) const {
        return t.data_type();
      }

      data_types::data_type operator()(const vector_type_info & t) const {
        return t.data_type();
      }

      template<typename Var>
      data_types::data_type operator()(const Var &) const {
        J_THROW("Unsupported variable type.");
      }
    };
  }

  shader_metadata shader_metadata::introspect(const u8_t * spirv, u32_t size) {
    J_ASSERT_NOT_NULL(spirv, size);
    J_ASSERT((size & 3) == 0, "Size not aligned.");
    J_ASSERT(size > sizeof(spirv_header), "Size does not accommodate header.");

    const spirv_header * header = reinterpret_cast<const spirv_header*>(spirv);
    shader_metadata result(header->version >> 16U, header->version);

    // const counts cnt = count_instructions(spirv, size);

    const instruction_iterator end{spirv + size};

    {
      instruction_visitor visitor{result};
      instruction_iterator it{spirv + sizeof(spirv_header)};
      while (it < end) {
        visit_instruction<insn_variable, insn_spec_constant,
                          insn_type_void, insn_type_bool, insn_type_int, insn_type_float, insn_type_vector,
                          insn_type_matrix, insn_type_image, insn_type_sampler, insn_type_sampled_image, insn_type_array,
                          insn_type_runtime_array, insn_type_struct, insn_type_pointer/*, insn_type_function*/>(visitor, *it++);
      }
    }

    {
      annotation_visitor visitor{result};
      instruction_iterator it{spirv + sizeof(spirv_header)};
      while (it < end) {
        visit_instruction<insn_name, insn_member_name, insn_decorate, insn_member_decorate>(visitor, *it++);
      }
    }
    return result;
  }

  vertex_data::vertex_input_info shader_metadata::get_vertex_input_info() const {
    vertex_data::vertex_input_info result;
    for (auto & v : vars) {
      auto & var = v.second;
      if (var.kind() != variable_kind::input) {
        continue;
      }
      auto & info = var.input_info();
      if (!info.has_location()) {
        continue;
      }
      const u8_t loc = info.location();
      J_REQUIRE(loc < vertex_data::vertex_input_info::max_locations_v, "Out of bounds");
      const auto & ptr = var.type().as_pointer();
      J_ASSERT(ptr.storage_class() == storage_class::input, "Ptr storage class mismatch");
      result[loc] = {
        ptr.target_type().visit(vertex_input_variable_visitor()),
        var.name(),
      };
    }
    return result;
  }

  specialization_constant_set shader_metadata::get_specialization_constant_set() const {
    specialization_constant_set result;
    for (auto & v : vars) {
      auto & var = v.second;
      if (var.kind() == variable_kind::spec_constant) {
        result.set(var.specialization_constant_info().id(),
                   var.type().visit(vertex_input_variable_visitor()),
                   var.name());
      }
    }
    return result;
  }
}
