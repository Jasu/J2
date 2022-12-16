#include "cli/cli.hpp"
#include "files/memory_mapping.hpp"
#include "strings/styling/default_styles.hpp"
#include "files/paths/path.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "rendering/vertex_data/vertex_input_info.hpp"
#include "rendering/vulkan/spirv/shader_metadata.hpp"

namespace j::rendering::vulkan::utils {
  namespace {
    namespace str = strings;
    namespace st = strings::styling::styles;

    J_NO_DEBUG const char * const type_styles[]{
      [(u8_t)type_kind::void_type] = "{#gray}{}{/}",
      [(u8_t)type_kind::boolean_type] = "{#yellow}{}{/}",
      [(u8_t)type_kind::integer_type] = "{#cyan}{}{/}",
      [(u8_t)type_kind::float_type] = "{#green}{}{/}",
      [(u8_t)type_kind::vector_type] = "{#bright_blue}{}{/}",
      [(u8_t)type_kind::matrix_type] = "{#bright_magenta}{}{/}",

      [(u8_t)type_kind::image_type] = "{#bright_green}{} image {}{/}",
      [(u8_t)type_kind::sampled_image_type] = "{#bright_green}Sampled {} image {}{/}",

      [(u8_t)type_kind::array_type] = "{#bright_cyan}Array{/} of",
      [(u8_t)type_kind::runtime_array_type] = "{#bright_cyan}Runtime array{/} of ",
    };

    struct J_TYPE_HIDDEN type_printer final {
      strings::formatted_sink & out;
      u32_t indent = 10U;


      void operator()(const basic_type_info_base & t) {
        out.write(type_styles[(u8_t)t.kind()], t.data_type());
      }

      void operator()(const image_type_info_base & t) {
        out.write(type_styles[(u8_t)t.kind()], t.dimensionality(), t.format());
      }

      void operator()(const array_type_info_base & t) {
        out.write(type_styles[(u8_t)t.kind()]);
        t.element_type().visit(*this);
      }

      void operator()(const struct_type_info & t) {
        if (t.name()) {
          out.write("Struct {#bright_yellow}{}{/}:", t.name());
        } else {
          out.write("Struct:");
        }
        indent += 2U;
        for (auto & i : t) {
          out.write("\n{indent}", indent);
          if (i.name()) {
            out.write("{#bright_green}{}{/} ", i.name());
          }
          out.write("{#bright_red}@{}{/}: ", i.offset());
          i.type().visit(*this);
        }
        indent -= 2U;
      }
      void operator()(const pointer_type_info & t) {
        out.write("{} pointer to ", t.storage_class());
        auto tgt = t.target_type();
        if (tgt.kind() == type_kind::struct_type) {
          auto & s = tgt.as_struct();
          if (s.name()) {
            out.write_styled(st::bright_yellow, s.name());
            return;
          }
        }
        tgt.visit(*this);
      }
    };

    shader_metadata introspect(strings::const_string_view path) {
      files::memory_mapping mapping{path, files::map_full_range};
      return shader_metadata::introspect(reinterpret_cast<const u8_t*>(mapping.data()), mapping.size());
    }

    void spirv_introspect(const mem::shared_ptr<strings::formatted_sink> & stdout,
                          const strings::string & path) {
      auto metadata = introspect(path);
      type_printer visitor{*stdout};
      for (auto & t :  metadata.types) {
        stdout->write("{#bold}Type {:3}:{/} ", t.first);
        t.second.visit(visitor);
        stdout->write("\n");
      }

      for (auto & v : metadata.vars) {
        i32_t index = v.first;
        auto & var = v.second;
        stdout->write("{#bold}{} variable #{} {}:{/}\n"
                      "{#bold}  Type: "
                      , var.kind(), index, var.name());
        var.type().visit(visitor);
        stdout->write("\n");
        if (var.kind() == variable_kind::input || var.kind() == variable_kind::output) {
          auto & info = var.kind() == variable_kind::input ? var.input_info() : var.output_info();
          if (info.has_location()) {
            stdout->write("{#bold}  Location: {}\n", info.location());
          }
          if (info.has_component()) {
            stdout->write("{#bold}  Component: {}\n", info.component());
          }
        }

        if (var.kind() == variable_kind::uniform) {
          auto & info = var.uniform_info();
          stdout->write("{#bold}  Binding: {}\n"
                        "{#bold}  Descriptor set: {}\n",
                        info.binding(), info.descriptor_set());
        }

        if (var.kind() == variable_kind::spec_constant) {
          auto & info = var.specialization_constant_info();
          stdout->write("{#bold}  Id: {}\n", info.id());
        }
      }

      u32_t i = 0;
      for (auto & v : metadata.get_vertex_input_info()) {
        stdout->write("{#bold}Vertex attribute #{}", i++);
        if (v.name) {
          stdout->write(" ({#bright_yellow}{})", v.name);
        }
        stdout->write(": {#bright_cyan}{}{/}\n", v.type);
      }
    }
  }

  namespace {
    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<c::command> cmd(
      "rendering.vulkan.spirv_introspect",
      "SPIR-V introspection command",

      c::command_name = "spirv-introspect",
      c::callback     = s::call(&spirv_introspect,
                                s::service("stdout"),
                                c::argument("path")));
  }
}
