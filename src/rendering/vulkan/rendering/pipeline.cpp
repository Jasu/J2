#include "rendering/vulkan/rendering/pipeline.hpp"

#include "rendering/vulkan/rendering/pipeline_description.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/rendering/render_pass.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/shaders/shader_module.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/surface_context.hpp"
#include "rendering/vulkan/descriptors/descriptor_set.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/spirv/specialization_constant_set.hpp"
#include "rendering/vertex_data/vertex_input_info.hpp"
#include "files/paths/path.hpp"
#include "files/fs.hpp"
#include "files/ofile.hpp"
#include "mem/shared_ptr.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "strings/whitelist.hpp"
#include "exceptions/assert.hpp"

static const VkPipelineRasterizationStateCreateInfo rasterizer_info{
  .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
  .depthClampEnable        = VK_FALSE,
  .rasterizerDiscardEnable = VK_FALSE,
  .polygonMode             = VK_POLYGON_MODE_FILL,
  .cullMode                = VK_CULL_MODE_NONE,
  .frontFace               = VK_FRONT_FACE_CLOCKWISE,
  .depthBiasEnable         = VK_FALSE,
  .lineWidth               = 1.0f,
};

// Dummy viewport - the viewport is dynamic.
static const VkViewport dummy_viewport{
  .x = 0.0f,
  .y = 0.0f,
  .width = 1.0f,
  .height = 1.0f,
  .minDepth = 0.0f,
  .maxDepth = 1.0f,
};

// Dummy scissor - the scissor test is dynamic.
static const VkRect2D dummy_scissor{
  .offset = { 0, 0 },
  .extent = { 1, 1 },
};

static const VkPipelineViewportStateCreateInfo viewport_info{
  .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
  .viewportCount = 1,
  .pViewports = &dummy_viewport,
  .scissorCount = 1,
  .pScissors = &dummy_scissor,
};

static const VkPipelineMultisampleStateCreateInfo multisample_info{
  .sType                   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
  .rasterizationSamples    = VK_SAMPLE_COUNT_1_BIT,
  .sampleShadingEnable     = VK_FALSE,
};

static const VkPipelineColorBlendAttachmentState color_blend_state{
  .blendEnable             = VK_TRUE,
  .srcColorBlendFactor     = VK_BLEND_FACTOR_SRC_ALPHA,
  .dstColorBlendFactor     = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
  .colorBlendOp            = VK_BLEND_OP_ADD,
  .srcAlphaBlendFactor     = VK_BLEND_FACTOR_ONE,
  .dstAlphaBlendFactor     = VK_BLEND_FACTOR_ZERO,
  .alphaBlendOp            = VK_BLEND_OP_ADD,
  .colorWriteMask          = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                           | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

static const VkPipelineColorBlendStateCreateInfo color_blend_state_info{
  .sType                   = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
  .logicOpEnable           = VK_FALSE,
  .logicOp                 = VK_LOGIC_OP_COPY,
  .attachmentCount         = 1,
  .pAttachments            = &color_blend_state,
  .blendConstants          = { 0.0f },
};

static const VkDynamicState dynamic_states[]{
  VK_DYNAMIC_STATE_VIEWPORT,
  VK_DYNAMIC_STATE_SCISSOR,
};

static const VkPipelineDynamicStateCreateInfo dynamic_state_info{
  .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
  .dynamicStateCount = J_ARRAY_SIZE(dynamic_states),
  .pDynamicStates = &dynamic_states[0],
};

namespace j::rendering::vulkan::rendering {
  namespace dt = j::rendering::data_types;
  namespace vd = vertex_data;
  namespace {
    VkFormat get_attribute_format(dt::data_type type, vd::scaling scaling, const char * pipeline, u32_t location) {
      switch (scaling) {
      case vd::scaling::scaled:
        switch (type) {
        case dt::data_type::u8:           return VK_FORMAT_R8_USCALED;
        case dt::data_type::s8:           return VK_FORMAT_R8_SSCALED;
        case dt::data_type::u16:          return VK_FORMAT_R16_USCALED;
        case dt::data_type::s16:          return VK_FORMAT_R16_SSCALED;

        case dt::data_type::vec2u8:      return VK_FORMAT_R8G8_USCALED;
        case dt::data_type::vec2s8:      return VK_FORMAT_R8G8_SSCALED;
        case dt::data_type::vec2u16:     return VK_FORMAT_R16G16_USCALED;
        case dt::data_type::vec2s16:     return VK_FORMAT_R16G16_SSCALED;

        case dt::data_type::vec3u8:      return VK_FORMAT_R8G8B8_USCALED;
        case dt::data_type::vec3s8:      return VK_FORMAT_R8G8B8_SSCALED;
        case dt::data_type::vec3u16:     return VK_FORMAT_R16G16B16_USCALED;
        case dt::data_type::vec3s16:     return VK_FORMAT_R16G16B16_SSCALED;

        case dt::data_type::vec4u8:      return VK_FORMAT_R8G8B8A8_USCALED;
        case dt::data_type::vec4s8:      return VK_FORMAT_R8G8B8A8_SSCALED;
        case dt::data_type::vec4u16:     return VK_FORMAT_R16G16B16A16_USCALED;
        case dt::data_type::vec4s16:     return VK_FORMAT_R16G16B16A16_SSCALED;
        default:
          J_THROW("Unsupported format for scaled data type {} for location #{} in pipeline {}", type, location, pipeline);
        }
      case vd::scaling::normalized:
        switch (type) {
        case dt::data_type::u8:       return VK_FORMAT_R8_UNORM;
        case dt::data_type::s8:       return VK_FORMAT_R8_SNORM;
        case dt::data_type::u16:      return VK_FORMAT_R16_UNORM;
        case dt::data_type::s16:      return VK_FORMAT_R16_SNORM;
        case dt::data_type::fp32:        return VK_FORMAT_R32_SFLOAT;

        case dt::data_type::vec2u8:  return VK_FORMAT_R8G8_UNORM;
        case dt::data_type::vec2s8:  return VK_FORMAT_R8G8_SNORM;
        case dt::data_type::vec2u16: return VK_FORMAT_R16G16_UNORM;
        case dt::data_type::vec2s16: return VK_FORMAT_R16G16_SNORM;
        case dt::data_type::vec2fp32:   return VK_FORMAT_R32G32_SFLOAT;

        case dt::data_type::vec3u8:  return VK_FORMAT_R8G8B8_UNORM;
        case dt::data_type::vec3s8:  return VK_FORMAT_R8G8B8_SNORM;
        case dt::data_type::vec3u16: return VK_FORMAT_R16G16B16_UNORM;
        case dt::data_type::vec3s16: return VK_FORMAT_R16G16B16_SNORM;
        case dt::data_type::vec3fp32:   return VK_FORMAT_R32G32B32_SFLOAT;

        case dt::data_type::vec4u8:  return VK_FORMAT_R8G8B8A8_UNORM;
        case dt::data_type::vec4s8:  return VK_FORMAT_R8G8B8A8_SNORM;
        case dt::data_type::vec4u16: return VK_FORMAT_R16G16B16A16_UNORM;
        case dt::data_type::vec4s16: return VK_FORMAT_R16G16B16A16_SNORM;
        case dt::data_type::vec4fp32:   return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
          J_THROW("Unsupported format for normalized data type {} for location #{} in pipeline {}", type, location, pipeline);
        }
      case vd::scaling::integer:
        switch (type) {
        case dt::data_type::u8:              return VK_FORMAT_R8_UINT;
        case dt::data_type::s8:              return VK_FORMAT_R8_SINT;
        case dt::data_type::u16:             return VK_FORMAT_R16_UINT;
        case dt::data_type::s16:             return VK_FORMAT_R16_SINT;
        case dt::data_type::u32:             return VK_FORMAT_R32_UINT;
        case dt::data_type::s32:             return VK_FORMAT_R32_SINT;

        case dt::data_type::vec2u8:         return VK_FORMAT_R8G8_UINT;
        case dt::data_type::vec2s8:         return VK_FORMAT_R8G8_SINT;
        case dt::data_type::vec2u16:        return VK_FORMAT_R16G16_UINT;
        case dt::data_type::vec2s16:        return VK_FORMAT_R16G16_SINT;
        case dt::data_type::vec2u32:        return VK_FORMAT_R32G32_UINT;
        case dt::data_type::vec2s32:        return VK_FORMAT_R32G32_SINT;

        case dt::data_type::vec3u8:         return VK_FORMAT_R8G8B8_UINT;
        case dt::data_type::vec3s8:         return VK_FORMAT_R8G8B8_SINT;
        case dt::data_type::vec3u16:        return VK_FORMAT_R16G16B16_UINT;
        case dt::data_type::vec3s16:        return VK_FORMAT_R16G16B16_SINT;
        case dt::data_type::vec3u32:        return VK_FORMAT_R32G32B32_UINT;
        case dt::data_type::vec3s32:        return VK_FORMAT_R32G32B32_SINT;

        case dt::data_type::vec4u8:         return VK_FORMAT_R8G8B8A8_UINT;
        case dt::data_type::vec4s8:         return VK_FORMAT_R8G8B8A8_SINT;
        case dt::data_type::vec4u16:        return VK_FORMAT_R16G16B16A16_UINT;
        case dt::data_type::vec4s16:        return VK_FORMAT_R16G16B16A16_SINT;
        case dt::data_type::vec4u32:        return VK_FORMAT_R32G32B32A32_UINT;
        case dt::data_type::vec4s32:        return VK_FORMAT_R32G32B32A32_SINT;
        default:
          J_THROW("Unsupported format for integer data type {} for location #{} in pipeline {}", type, location, pipeline);
        }
      }
    }

    u32_t get_spec_constant_size(const trivial_array<attributes::spec_constant_value> & consts) noexcept {
      u32_t result = 0U;
      for (auto & spec : consts) {
        result += spec.value.size();
      }
      return result;
    }

    void populate_spec_consts(
      const trivial_array<attributes::spec_constant_value> & consts,
      const specialization_constant_set & set,
      VkSpecializationMapEntry * entries,
      u8_t * buffer)
    {
      u32_t offset = 0U;
      for (auto & c : consts) {
        const auto sz = c.value.size();
        entries->constantID = set.index_of(c.key);
        entries->offset = offset;
        entries->size = sz;
        ::j::memcpy(buffer, c.value.data(), sz);
        offset += sz;
        buffer += sz;
        ++entries;
      }
    }

  }
  pipeline::pipeline(const surface_context & context,
                     pipeline_description && description)
    : push_constants(static_cast<trivial_array<VkPushConstantRange> &&>(
                       description.push_constants)),
      name(description.name),
      vertex_shader(static_cast<shaders::shader_module_ref &&>(description.vertex_shader.shader)),
      fragment_shader(static_cast<shaders::shader_module_ref &&>(description.fragment_shader.shader)),
      device(context.device_ref())
  {
    J_ASSERT_NOT_NULL(vertex_shader, fragment_shader);
    const VkDevice vk_device = context.device().vk_device;

    // Descriptor set layouts:
    // - There may be multiple descriptor set layouts, for e.g. `layout(set = 0, binding = 0)`.
    // - The description has either layouts or descriptor sets, since the API consumer should not
    //   need to think about descriptor set layouts.
    {
      const u32_t num_layouts = description.descriptor_set_layouts.size();
      J_ASSERT(num_layouts < 4, "Layout count out of range");
      VkDescriptorSetLayout set_layouts[num_layouts];
      u32_t i = 0;
      for (auto & l : description.descriptor_set_layouts) {
        if (l.is_descriptor_set_layout()) {
          m_descriptor_set_layouts[i] = static_cast<descriptors::descriptor_set_layout_ref &&>(l.as_descriptor_set_layout());
        } else {
          m_descriptor_set_layouts[i] = context.get(l.as_descriptor_set()).layout;
        }
        set_layouts[i] = context.get(m_descriptor_set_layouts[i]).vk_descriptor_set_layout;
        ++i;
      }

      const VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = num_layouts,
        .pSetLayouts = set_layouts,
        .pushConstantRangeCount = (u32_t)push_constants.size(),
        .pPushConstantRanges = push_constants.begin(),
      };

      J_VK_CALL_CHECKED(vkCreatePipelineLayout, vk_device,
                        &pipeline_layout_info, nullptr, &vk_pipeline_layout);
    }



    const VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology                = description.primitive_topology,
      .primitiveRestartEnable  = VK_FALSE,
    };

    // J_ASSERT(!description.vertex_input_state.bindings.empty(), "No vertex input bindings.");
    u32_t num_vertex_attributes = 0;
    for (auto & b : description.vertex_input_state.bindings) {
      num_vertex_attributes += b.attributes.size();
    }
    VkVertexInputBindingDescription descs[description.vertex_input_state.bindings.size()];
    VkVertexInputAttributeDescription attrs[num_vertex_attributes];
    u32_t desc_index = 0U, attr_index = 0U;
    for (auto & b : description.vertex_input_state.bindings) {
      vertex_data::matched_vertex_binding_info matched(b.attributes, context.get(vertex_shader).get_vertex_input_info());
      for (auto it = matched.begin(), end = matched.end(); it != end; ++it) {
        attrs[attr_index] = VkVertexInputAttributeDescription{
          .location = it.index,
          .binding = desc_index,
          .format = get_attribute_format(it->type, it->scaling, name, attr_index),
          .offset = it->offset,
        };
        ++attr_index;
      }
      descs[desc_index] = VkVertexInputBindingDescription{
        .binding = desc_index,
        .stride = (u32_t)b.attributes.stride(),
        .inputRate = b.is_instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX,
      };
      desc_index++;
    }

    const VkPipelineVertexInputStateCreateInfo vertex_input_state_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = description.vertex_input_state.bindings.size(),
      .pVertexBindingDescriptions = descs,
      .vertexAttributeDescriptionCount = num_vertex_attributes,
      .pVertexAttributeDescriptions = attrs,
    };

    const auto vertex_specs = context.get(vertex_shader).specialization_constants();
    const auto frag_specs = context.get(fragment_shader).specialization_constants();
    const u32_t num_vertex_consts = description.vertex_shader.specialization_constants.size();
    const u32_t vertex_spec_size = get_spec_constant_size(description.vertex_shader.specialization_constants);
    const u32_t num_frag_consts = description.fragment_shader.specialization_constants.size();
    const u32_t frag_spec_size = get_spec_constant_size(description.fragment_shader.specialization_constants);

    VkSpecializationMapEntry vertex_spec_entries[num_vertex_consts];
    VkSpecializationMapEntry frag_spec_entries[num_frag_consts];
    u8_t vertex_spec_buffer[vertex_spec_size];
    u8_t frag_spec_buffer[frag_spec_size];

    populate_spec_consts(description.vertex_shader.specialization_constants, vertex_specs,
                         vertex_spec_entries, vertex_spec_buffer);
    populate_spec_consts(description.fragment_shader.specialization_constants, frag_specs,
                         frag_spec_entries, frag_spec_buffer);

    const VkSpecializationInfo spec[]{
      {
        .mapEntryCount = num_vertex_consts,
        .pMapEntries = vertex_spec_entries,
        .dataSize = vertex_spec_size,
        .pData = vertex_spec_buffer,
      },
      {
        .mapEntryCount = num_frag_consts,
        .pMapEntries = frag_spec_entries,
        .dataSize = frag_spec_size,
        .pData = frag_spec_buffer,
      },
    };

    const VkPipelineShaderStageCreateInfo stages[]{
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = context.get(vertex_shader).vk_shader_module,
        .pName = "main",
        .pSpecializationInfo = &spec[0],
      },
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = context.get(fragment_shader).vk_shader_module,
        .pName = "main",
        .pSpecializationInfo = &spec[1],
      },
    };

    const VkGraphicsPipelineCreateInfo pipeline_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .flags = context.instance().configuration().dump_shaders
        ? VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR
        | VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR
        : 0U,
      .stageCount = 2,
      .pStages = stages,
      .pVertexInputState = &vertex_input_state_info,
      .pInputAssemblyState = &input_assembly_info,
      .pViewportState = &viewport_info,
      .pRasterizationState = &rasterizer_info,
      .pMultisampleState = &multisample_info,
      .pColorBlendState = &color_blend_state_info,
      .pDynamicState = &dynamic_state_info,
      .layout = vk_pipeline_layout,
      .renderPass = context.render_pass().vk_render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
    };

    try {
      J_VK_CALL_CHECKED(vkCreateGraphicsPipelines, vk_device, nullptr,
                        1, &pipeline_info, nullptr, &vk_pipeline);
    } catch (...) {
      vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);
      vk_pipeline_layout = VK_NULL_HANDLE;
      throw;
    }

    context.set_object_name(vk_pipeline, name);
    if (context.instance().configuration().dump_shaders) {
      debug_dump_pipeline("./pipeline_dump");
    }
  }

  span<const descriptors::descriptor_set_layout_ref> pipeline::descriptor_set_layouts() const noexcept {
    u32_t num = 1U;
    for (; m_descriptor_set_layouts[num] && num < 4; ++num) { }
    return span<const descriptors::descriptor_set_layout_ref>(
      m_descriptor_set_layouts, num);
  }

  pipeline::~pipeline() {
    if (auto dev = device.lock()) {
      const VkDevice d = dev.get().vk_device;
      vkDestroyPipeline(d, vk_pipeline, nullptr);
      vkDestroyPipelineLayout(d, vk_pipeline_layout, nullptr);
    }
    vk_pipeline = VK_NULL_HANDLE, vk_pipeline_layout = VK_NULL_HANDLE;
  }

  const VkPushConstantRange & pipeline::get_push_constant_range(u8_t offset) const {
    for (const VkPushConstantRange & r : push_constants) {
      if (r.offset == offset) {
        return r;
      }
    }
    J_THROW("Push constant offset not found.");
  }

  void pipeline::debug_dump_pipeline(const files::path & path) {
    auto d = device.lock().get().vk_device;
    J_ASSERT_NOT_NULL(d, path, vk_pipeline);
    if (!files::fs::exists(path)) {
      files::fs::mkdir(path);
    }
    strings::string txt_filename = name;
    txt_filename += "-statistics.txt";
    txt_filename = strings::whitelist(txt_filename,
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "1234567890-_.",
                                      '-');
    auto stats_file = strings::formatted_sink(
      mem::static_pointer_cast<streams::sink>(
        mem::make_shared<files::ofile>(path / txt_filename)));

    const VkPipelineInfoKHR pipeline_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR,
      .pipeline = vk_pipeline,
    };

    auto fn_vkGetPipelineExecutablePropertiesKHR =
      (PFN_vkGetPipelineExecutablePropertiesKHR)vkGetDeviceProcAddr(
      d, "vkGetPipelineExecutablePropertiesKHR");
    auto fn_vkGetPipelineExecutableInternalRepresentationsKHR =
      (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)vkGetDeviceProcAddr(
      d, "vkGetPipelineExecutableInternalRepresentationsKHR");
    auto fn_vkGetPipelineExecutableStatisticsKHR =
      (PFN_vkGetPipelineExecutableStatisticsKHR)vkGetDeviceProcAddr(
      d, "vkGetPipelineExecutableStatisticsKHR");

    u32_t num_executables = 0U;
    J_VK_CALL_CHECKED(fn_vkGetPipelineExecutablePropertiesKHR, d,
                      &pipeline_info, &num_executables, nullptr);

    VkPipelineExecutablePropertiesKHR executables[num_executables];

    ::j::memset(executables, 0, sizeof(executables));
    for (u32_t i = 0; i < num_executables; ++i) {
      executables[i].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
    }

    J_VK_CALL_CHECKED(fn_vkGetPipelineExecutablePropertiesKHR, d,
                      &pipeline_info, &num_executables, executables);

    for (u32_t i = 0; i < num_executables; ++i) {
      stats_file.write("Statistics for {}:\n{}\n\n", (const char *)executables[i].name, (const char *)executables[i].description);
      const VkPipelineExecutableInfoKHR executable_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR,
        .pipeline = vk_pipeline,
        .executableIndex = i,
      };

      u32_t num_stats = 0U;
      J_VK_CALL_CHECKED(fn_vkGetPipelineExecutableStatisticsKHR, d,
                        &executable_info, &num_stats, nullptr);

      VkPipelineExecutableStatisticKHR stats[num_stats];
      ::j::memset(stats, 0, sizeof(stats));
      for (u32_t j = 0; j < num_stats; ++j) {
        stats[j].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
      }

      J_VK_CALL_CHECKED(fn_vkGetPipelineExecutableStatisticsKHR, d,
                        &executable_info, &num_stats, stats);

      for (u32_t j = 0; j < num_stats; ++j) {
        stats_file.write("{}: ", (const char*)stats[j].name);
        switch (stats[j].format) {
        case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
          stats_file.write("{}", (bool)stats[j].value.b32);
          break;
        case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
          stats_file.write("{}", stats[j].value.i64);
          break;
        case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
          stats_file.write("{}", stats[j].value.u64);
          break;
        case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
          stats_file.write("{}", stats[j].value.f64);
          break;
        case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_MAX_ENUM_KHR:
          break;
        }
        stats_file.write("\n{}\n\n", (const char*)stats[j].description);
      }

      u32_t num_irs = 0U;
      J_VK_CALL_CHECKED(fn_vkGetPipelineExecutableInternalRepresentationsKHR, d,
                        &executable_info, &num_irs, nullptr);
      VkPipelineExecutableInternalRepresentationKHR irs[num_irs];
      ::j::memset(irs, 0, sizeof(irs));
      for (u32_t j = 0; j < num_irs; ++j) {
        irs[j].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR;
        irs[j].dataSize = 32 * 1024;
        irs[j].pData = ::j::allocate(irs[j].dataSize);
      }
      J_VK_CALL_CHECKED(fn_vkGetPipelineExecutableInternalRepresentationsKHR, d,
                        &executable_info, &num_irs, irs);

      for (u32_t j = 0; j < num_irs; ++j) {
        J_VK_INFO("  IR {} {}", (const char *)irs[j].name, (const char *)irs[j].description);
        if (!irs[j].pData) {
          J_VK_INFO("  No data");
          continue;
        }
        strings::string filename = name;
        filename += "-";
        filename += (const char *)executables[i].name;
        filename += "-";
        filename += (const char *)irs[j].name;
        filename = strings::whitelist(filename,
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "1234567890-_.",
                                      '-');
        J_ASSERT_NOT_NULL(filename);
        J_ASSERT(filename != "." && filename != "..", "Invalid filename");
        files::ofile f(path / filename);
        f.write_bytes(static_cast<const char *>(irs[j].pData), irs[j].dataSize);
        ::j::free(irs[j].pData);
      }
    }
  }

  DEFINE_SURFACE_RESOURCE_DEFINITION(pipeline, pipeline_description, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::pipeline, j::rendering::vulkan::rendering::pipeline_description);
