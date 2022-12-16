#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/configuration.hpp"
#include "rendering/vulkan/utils.hpp"
#include "strings/string.hpp"

#include "services/services.hpp"
#include "application/information.hpp"
#include "logging/logging.hpp"
#include "hzd/string.hpp"

namespace j::rendering::vulkan {
  namespace {
    constexpr const char * const validation_layers[] = {
      "VK_LAYER_KHRONOS_validation",
    };

    constexpr const char * const extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_xcb_surface",
      "VK_KHR_get_physical_device_properties2",
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    constexpr u32_t num_debug_extensions = 1;
  }

  instance instance::singleton;

  instance::instance() noexcept {
  }

  instance::~instance() {
    if (m_vk_instance) {
      finalize();
    }
  }

  void instance::initialize(mem::shared_ptr<j::logging::logger> logger,
                            mem::shared_ptr<j::application::information> app,
                            const struct configuration & conf)
  {
    J_ASSERT_NOT_NULL(logger, app);

    m_configuration = conf;

    m_logger = static_cast<mem::shared_ptr<j::logging::logger> &&>(logger);

    J_VK_INFO("Initializing Vulkan");

    const u32_t version = VK_MAKE_VERSION(app->major_version, app->minor_version, app->patch_version);
    strings::string app_name{app->application_name.size() + 1};
    memcpy(app_name.data(), app->application_name.data(), app->application_name.size());
    app_name[app->application_name.size()] = 0;

    const VkApplicationInfo application_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = app_name.data(),
      .applicationVersion = version,
      .pEngineName = "j",
      .engineVersion = version,
      .apiVersion = VK_API_VERSION_1_1,
    };

    const VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &application_info,
      .enabledLayerCount = static_cast<u32_t>(conf.enable_debug ? J_ARRAY_SIZE(validation_layers) : 0),
      .ppEnabledLayerNames = validation_layers,
      .enabledExtensionCount = static_cast<u32_t>(J_ARRAY_SIZE(extensions) - (conf.enable_debug ? 0 : num_debug_extensions)),
      .ppEnabledExtensionNames = extensions,
    };
    J_VK_CALL_CHECKED(vkCreateInstance, &create_info, nullptr, &m_vk_instance);
  }

  void instance::finalize() noexcept {
    J_VK_INFO("Finalizing Vulkan instance.");
    J_ASSERT_NOT_NULL(m_vk_instance); // Double-finalize
    vkDestroyInstance(m_vk_instance, nullptr);
    m_vk_instance = nullptr;
  }

  void instance::log_warning(const strings::string & message) {
    m_logger->warning(true, strings::const_string_view(message));
  }

  void instance::log_error(const strings::string & message) {
    m_logger->error(true, strings::const_string_view(message));
  }

  void instance::log_info(const strings::string & message) {
    m_logger->info(true, strings::const_string_view(message));
  }

  void instance::log_notice(const strings::string & message) {
    m_logger->notice(true, strings::const_string_view(message));
  }

  void instance::log_debug(const strings::string & message) {
    m_logger->debug(true, strings::const_string_view(message));
  }

  void instance::log_warning(const char * message) {
    m_logger->warning(true, message);
  }

  void instance::log_error(const char * message) {
    m_logger->error(true, message);
  }

  void instance::log_info(const char * message) {
    m_logger->info(true, message);
  }

  void instance::log_notice(const char * message) {
    m_logger->notice(true, message);
  }

  void instance::log_debug(const char * message) {
    m_logger->debug(true, message);
  }

  namespace {
    using namespace services;
    J_A(ND, NODESTROY) service_definition<instance> vulkan_instance_definition(
      "vulkan.instance",
      "Vulkan instance",
      setting = setting_object<vulkan::configuration>("configuration"),
      create = &instance::singleton,
      initialize = initializer_call(
        &instance::initialize,
        arg::autowire,
        arg::autowire,
        arg::setting("configuration")),
      finalize = &instance::finalize
    );
  }
}
