#include "rendering/vulkan/shaders/shader_module.hpp"

#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/spirv/specialization_constant_set.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vertex_data/vertex_input_info.hpp"
#include "files/memory_mapping.hpp"
#include "files/paths/path.hpp"

namespace j::rendering::vulkan::shaders {
  shader_module::shader_module(const device_context_base & context,
                               const files::path & path)
    : device(context.device_ref())
  {
    files::memory_mapping mapping{path, files::map_full_range};
    metadata = shader_metadata::introspect((const u8_t*)mapping.data(), mapping.size());

    const VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (sz_t)mapping.size(),
      .pCode = reinterpret_cast<const u32_t*>(mapping.data()),
    };
    J_VK_CALL_CHECKED(vkCreateShaderModule, context.device().vk_device,
                      &info, nullptr, &vk_shader_module);

    auto basename = path.basename();
    context.set_object_name(vk_shader_module, basename.data());
  }

  shader_module::~shader_module() {
    if (auto dev = device.lock()) {
      vkDestroyShaderModule(dev.get().vk_device, vk_shader_module, nullptr);
    }
  }

  specialization_constant_set shader_module::specialization_constants() const {
    return metadata.get_specialization_constant_set();
  }

  vertex_data::vertex_input_info shader_module::get_vertex_input_info() const {
    return metadata.get_vertex_input_info();
  }
}
