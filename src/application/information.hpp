#pragma once

#include "services/services.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"
#include "files/paths/path.hpp"

namespace j::application {
  /**
   * Configuration that must be present before configuration is loaded.
   */
  struct information {
    /**
     * Name of the applications used for technical purposes.
     *
     * Used for:
     * - XDG directory names,
     * - /etc/ directory name, and
     * - application name for Vulkan.
     */
    strings::string application_name;

    u8_t major_version;
    u8_t minor_version;
    u8_t patch_version;

    /**
     * Paths to configuration files.
     *
     * Each of these files will be resolved from the XDG configuration directory
     * and from the /etc/ directory (unless the paths are absolute.)
     */
    vector<files::path> configuration_files;
  };
}

#define J_APPLICATION_INFORMATION(...)                                       \
  namespace {                                                                \
    using namespace ::j::services;                                           \
    ::j::application::information _j_application_information{ __VA_ARGS__ }; \
    J_A(ND, NODESTROY) service_definition<::j::application::information>                        \
      _j_application_information_definition(                                 \
        "application.information",                                           \
        "Application information",                                           \
        create = &_j_application_information                                 \
      );                                                                     \
  }
