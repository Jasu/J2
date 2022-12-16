#pragma once

#include <vulkan/vulkan.h>
#include "hzd/utility.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::synchronization {
  struct fence final {
    explicit fence(const device_context_base & context);

    void finalize(const device_context_base & context);

    /// Waits for the fence (regularly, not by FD)
    bool wait(const device_context_base & context, u64_t timeout_ns = U64_MAX);

    /// Reset the fence.
    void reset(const device_context_base & context);

    /// Waits for the fence (regularly, not by FD) and then resets the fence.
    void wait_and_reset(const device_context_base & context, u64_t timeout_ns = U64_MAX);

    [[nodiscard]] int update_fd(const device_context_base & context);

    /// Checks if the fence is in signaled state.
    ///
    /// After polling for the file descriptor returns, this function MUST be called.
    [[nodiscard]] bool is_signaled(const device_context_base & context) const;

    VkFence vk_fence;
    /// The waitable file descriptor for this fence.
    ///
    /// When polling the FD, is_signaled MUST be called afterwards.
    int fd;

  private:
    VkFenceGetFdInfoKHR m_get_fd_info;
  };
}
