#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/exception.hpp"
#include "logging/logging.hpp"

#include "rendering/vulkan/instance.hpp"

namespace j::rendering::vulkan::detail {
  [[noreturn]] void throw_exception(const char * msg) {
    J_THROW(exception() << service_name("Vulkan") << message(msg));
  }

  [[noreturn]] void fail_function(const char *name, VkResult result) {
    J_THROW(exception() << service_name("Vulkan") << function_name(name) << vulkan_result(result));
  }

  void log_warning(strings::const_string_view message) {
    instance::singleton.log_warning(message);
  }

  void log_error(strings::const_string_view message) {
    instance::singleton.log_error(message);
  }

  void log_info(strings::const_string_view message) {
    instance::singleton.log_info(message);
  }

  void log_notice(strings::const_string_view message) {
    instance::singleton.log_notice(message);
  }

  void log_debug(strings::const_string_view message) {
    instance::singleton.log_debug(message);
  }

  void do_log(logging::severity sev, const char * message, u32_t num_params, const strings::formatting::format_value * params) {
    instance::singleton.logger().do_log(sev, true, message, num_params, params);
  }

  void do_log(logging::severity sev, strings::const_string_view message, u32_t num_params, const strings::formatting::format_value * params) {
    instance::singleton.logger().do_log(sev, true, strings::const_string_view(message), num_params, params);
  }
}
