#pragma once

#include "signals/signal.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  struct command_buffer_builder;
}

namespace j::rendering::vulkan::rendering {
  /// Abstract base class for scenes.
  ///
  /// A scene is basically a command buffer to execute, along with uniforms and other
  /// resources used during rendering of that scene.
  struct scene {
    J_BOILERPLATE(scene, CTOR_NE, MOVE_DEL, COPY_DEL)

    virtual ~scene();

    virtual void build_command_buffer(command_buffers::command_buffer_builder & builder) = 0;

    void invalidate() {
      ++stamp;
      on_invalidate();
    }

    signals::signal<void ()> on_invalidate;

    /// Stamp that something in the scene has changed and it should be re-rendered.
    u32_t stamp = 0;
  };
}
