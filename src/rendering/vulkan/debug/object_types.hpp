#pragma once

#include <vulkan/vulkan.h>

namespace j::rendering::vulkan::inline debug {
  template<typename Handle> struct vk_object_type;

  template<> struct vk_object_type<VkInstance>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_INSTANCE; };
  template<> struct vk_object_type<VkPhysicalDevice>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_PHYSICAL_DEVICE; };
  template<> struct vk_object_type<VkDevice>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_DEVICE; };
  template<> struct vk_object_type<VkQueue>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_QUEUE; };
  template<> struct vk_object_type<VkSemaphore>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_SEMAPHORE; };
  template<> struct vk_object_type<VkCommandBuffer>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_COMMAND_BUFFER; };
  template<> struct vk_object_type<VkFence>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_FENCE; };
  template<> struct vk_object_type<VkDeviceMemory>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_DEVICE_MEMORY; };
  template<> struct vk_object_type<VkBuffer>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_BUFFER; };
  template<> struct vk_object_type<VkImage>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_IMAGE; };
  template<> struct vk_object_type<VkEvent>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_EVENT; };
  template<> struct vk_object_type<VkQueryPool>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_QUERY_POOL; };
  template<> struct vk_object_type<VkBufferView>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_BUFFER_VIEW; };
  template<> struct vk_object_type<VkImageView>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_IMAGE_VIEW; };
  template<> struct vk_object_type<VkShaderModule>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_SHADER_MODULE; };
  template<> struct vk_object_type<VkPipelineCache>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_PIPELINE_CACHE; };
  template<> struct vk_object_type<VkPipelineLayout>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_PIPELINE_LAYOUT; };
  template<> struct vk_object_type<VkRenderPass>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_RENDER_PASS; };
  template<> struct vk_object_type<VkPipeline>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_PIPELINE; };
  template<> struct vk_object_type<VkDescriptorSetLayout>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
  template<> struct vk_object_type<VkSampler>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_SAMPLER; };
  template<> struct vk_object_type<VkDescriptorPool>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
  template<> struct vk_object_type<VkDescriptorSet>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
  template<> struct vk_object_type<VkFramebuffer>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_FRAMEBUFFER; };
  template<> struct vk_object_type<VkCommandPool>
  { static constexpr inline VkObjectType value = VK_OBJECT_TYPE_COMMAND_POOL; };

  template<typename Handle>
  constexpr inline VkObjectType vk_object_type_v = vk_object_type<Handle>::value;
}
