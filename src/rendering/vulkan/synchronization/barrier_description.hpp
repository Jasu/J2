#pragma once

#include <vulkan/vulkan.h>
#include "attributes/attribute_definition.hpp"
#include "attributes/enable_if_attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::rendering::vulkan::synchronization {
  /// What to wait for / what to make wait - both execution and memory.
  struct wait_type {
    /// Pipeline stages whose execution must complete before the barrier is released.
    ///
    /// Specifying separate wait and postpone stage flags mostly makes sense when using top-of-pipe
    /// or bottom-of-pipe to indicate that the wait should be a no-op.
    VkPipelineStageFlags wait_for_stage_flags = 0;
    /// Pipeline stages whose execution must not begin before the barrier is released.
    ///
    /// Specifying separate wait and postpone stage flags mostly makes sense when using top-of-pipe
    /// or bottom-of-pipe to indicate that the wait should be a no-op.
    VkPipelineStageFlags postpone_stage_flags = 0;
    /// Memory access type of the wait.
    VkAccessFlags access_flags = 0;

    J_ALWAYS_INLINE constexpr wait_type(VkPipelineStageFlags stage, VkAccessFlags access) noexcept
      : wait_for_stage_flags(stage),
        postpone_stage_flags(stage),
        access_flags(access)
    {
    }

    J_ALWAYS_INLINE constexpr wait_type(VkPipelineStageFlags wait_for_stage,
                                        VkPipelineStageFlags postpone_stage,
                                        VkAccessFlags access) noexcept
      : wait_for_stage_flags(wait_for_stage),
        postpone_stage_flags(postpone_stage),
        access_flags(access)
    {
    }
  };

  inline constexpr auto wait_for = j::attributes::attribute_definition(
    j::attributes::value_type = type<wait_type>,
    j::attributes::tag = type<struct wait_for_tag>);

  inline constexpr auto postpone = j::attributes::attribute_definition(
    j::attributes::value_type = type<wait_type>,
    j::attributes::tag = type<struct postpone_tag>);

  /// When specified as [wait_for], nothing will be waited.
  /// When specified as [postpone], nothing will be blocked.
  J_NO_DEBUG inline constexpr wait_type nothing{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                                0};

  /// When specified as [wait_for], wait that prior transfer writes to GPU memory complete.
  /// When specified as [postpone], make following transfer writes to GPU memory wait.
  J_NO_DEBUG inline constexpr wait_type transfer_write{
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_ACCESS_TRANSFER_WRITE_BIT};

  /// When specified as [wait_for], wait that prior transfer reads from GPU memory complete.
  /// When specified as [postpone], make following transfer reads from GPU memory wait.
  J_NO_DEBUG inline constexpr wait_type transfer_read{
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_ACCESS_TRANSFER_READ_BIT};

  /// When specified as [wait_for], wait that both transfer reads and writes complete.
  /// When specified as [postpone], make following transfer reads and writes wait.
  J_NO_DEBUG inline constexpr wait_type transfer{
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT};

  /// When specified as [wait_for], wait for prior fragment shader reads to complete.
  /// When specified as [postpone], make following fragment shader reads wait.
  J_NO_DEBUG inline constexpr wait_type fragment_shader_read{
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    VK_ACCESS_SHADER_READ_BIT};

  /// When specified as [wait_for], wait for prior fragment shader writes to complete.
  /// When specified as [postpone], make following fragment shader writes wait.
  J_NO_DEBUG inline constexpr wait_type fragment_shader_write{
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    VK_ACCESS_SHADER_WRITE_BIT};

  /// When specified as [wait_for], wait for prior fragment shader reads and writes to complete.
  /// When specified as [postpone], make following fragment shader reads and writes wait.
  J_NO_DEBUG inline constexpr wait_type fragment_shader{
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT};

  /// When specified as [wait_for], wait for prior vertex shader reads to complete.
  /// When specified as [postpone], make following vertex shader reads wait.
  J_NO_DEBUG inline constexpr wait_type vertex_shader_read{
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    VK_ACCESS_SHADER_READ_BIT};

  /// When specified as [wait_for], wait for prior vertex shader writes to complete.
  /// When specified as [postpone], make following vertex shader writes wait.
  J_NO_DEBUG inline constexpr wait_type vertex_shader_write{
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    VK_ACCESS_SHADER_WRITE_BIT};

  /// When specified as [wait_for], wait for prior vertex shader reads and writes to complete.
  /// When specified as [postpone], make following vertex shader reads and writes wait.
  J_NO_DEBUG inline constexpr wait_type vertex_shader{
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT};

  /// When specified as [wait_for], wait for prior host reads from GPU memory to complete.
  /// When specified as [postpone], make following host reads from GPU memory wait.
  ///
  /// Since [vkSubmitQueue] inserts an implicit host memory barrier, this is only needed
  /// when host-GPU interaction is synchronized within the same submission, e.g. by events.
  J_NO_DEBUG inline constexpr wait_type host_read{
    VK_PIPELINE_STAGE_HOST_BIT,
    VK_ACCESS_HOST_READ_BIT};

  /// When specified as [wait_for], wait for prior host writes to GPU memory to complete.
  /// When specified as [postpone], make following host writes to GPU memory wait.
  ///
  /// Since [vkSubmitQueue] inserts an implicit host memory barrier, this is only needed
  /// when host-GPU interaction is synchronized within the same submission, e.g. by events.
  J_NO_DEBUG inline constexpr wait_type host_write{
    VK_PIPELINE_STAGE_HOST_BIT,
    VK_ACCESS_HOST_WRITE_BIT};

  /// When specified as [wait_for], wait for prior host reads and writes to complete.
  /// When specified as [postpone], make following host reads and writes wait.
  ///
  /// Since [vkSubmitQueue] inserts an implicit host memory barrier, this is only needed
  /// when host-GPU interaction is synchronized within the same submission, e.g. by events.
  J_NO_DEBUG inline constexpr wait_type host{
    VK_PIPELINE_STAGE_HOST_BIT,
    VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT};

  /// Base class for pipeline barriers.
  class barrier_description {
  protected:
    /// Source wait, i.e. what to wait for before releasing the barrier.
    wait_type m_wait_for = nothing;
    /// Destination wait, i.e. what to block until the barrier is lifted.
    wait_type m_postpone = nothing;

  public:
    J_BOILERPLATE(barrier_description, CTOR_CE, COPY_DEL, MOVE_DEL)

    template<typename... Args>
    J_ALWAYS_INLINE explicit constexpr barrier_description(Args && ... args) noexcept {
      if constexpr(::j::attributes::has<Args...>(wait_for)) {
        m_wait_for = wait_for.get(static_cast<Args &&>(args)...);
      }
      if constexpr(::j::attributes::has<Args...>(postpone)) {
        m_postpone = postpone.get(static_cast<Args &&>(args)...);
      }
    }

    J_INLINE_GETTER VkAccessFlags wait_for_access_flags() const noexcept {
      return m_wait_for.access_flags;
    }

    /// Get source stage flags, i.e. the pipeline stages to wait for.
    J_INLINE_GETTER VkPipelineStageFlags wait_for_stage_flags() const noexcept {
      return m_wait_for.wait_for_stage_flags;
    }

    J_INLINE_GETTER VkAccessFlags postpone_access_flags() const noexcept {
      return m_postpone.access_flags;
    }

    /// Get destination stage flags, i.e. the pipeline stages to make wait for the barrier.
    J_INLINE_GETTER VkPipelineStageFlags postpone_stage_flags() const noexcept {
      return m_postpone.postpone_stage_flags;
    }
  };
}
