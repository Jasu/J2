#include "rendering/vulkan/configuration.hpp"
#include "properties/class_registration.hpp"
#include "cli/metadata.hpp"

namespace j::rendering::vulkan {
  namespace {
    namespace p = properties;
    const p::object_access_registration<configuration> reg(
      "vulkan_configuration",
      p::property = p::member<&j::rendering::vulkan::configuration::enable_debug>(
        "enable_debug",
        cli::cli_option = "vulkan-debug",
        cli::short_description = "Enable Vulkan debug layers and logging"
      ),
      p::property = p::member<&j::rendering::vulkan::configuration::dump_shaders>(
        "dump_shaders",
        cli::cli_option = "dump-shaders",
        cli::short_description = "Dump shader assembly to a subdirectory"
      ),
    p::property = p::member<&j::rendering::vulkan::configuration::dump_graphviz_on_right_click>(
      "dump_graphviz_on_right_click",
      cli::cli_option = "dump-graphviz-on-right-click",
      cli::short_description = "Dump renderer task graph into dump.dot on right-click"
    ));
  }
}
