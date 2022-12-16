#pragma once

namespace j::rendering::vulkan {
  struct configuration {
    /// When set, debugging layers are enabled.
    bool enable_debug = false;
    /// When set, shaders are dumped using VkPipelineExecutableInfoKHR.
    bool dump_shaders = false;
    /// When set, GraphViz will be dumped on right-click.
    bool dump_graphviz_on_right_click = false;
  };
}
