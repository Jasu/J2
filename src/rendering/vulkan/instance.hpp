#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/configuration.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::logging {
  class logger;
}

namespace j::application {
  struct information;
}
namespace j::strings {
  class string;
}

namespace j::rendering::vulkan {
  /// Represents the Vulkan instance.
  ///
  /// This is an application-wide singleton. In addition to wrapping [VkInstance], this class
  /// provides logging and common error facilities to all Vulkan code.
  class instance {
    VkInstance m_vk_instance = VK_NULL_HANDLE;
    mem::shared_ptr<j::logging::logger> m_logger;

  public:
    static instance singleton;

    instance() noexcept;

    ~instance();

    void initialize(mem::shared_ptr<j::logging::logger> logger,
                    mem::shared_ptr<j::application::information> app,
                    const configuration & conf);

    void finalize() noexcept;

    const struct configuration & configuration() const noexcept {
      return m_configuration;
    }

    inline VkInstance vk_instance() const noexcept {
      return m_vk_instance;
    }

    void log_warning(const strings::string & message);

    void log_error(const strings::string & message);

    void log_info(const strings::string & message);

    void log_notice(const strings::string & message);

    void log_debug(const strings::string & message);

    void log_warning(const char * message);

    void log_error(const char * message);

    void log_info(const char * message);

    void log_notice(const char * message);

    void log_debug(const char * message);

    j::logging::logger & logger() noexcept {
      return *m_logger;
    }
  private:
    struct configuration m_configuration;
  };
}

#define J_VK_INSTANCE (::j::rendering::vulkan::instance::singleton.vk_instance())
