#include "temp_file.hpp"

#include "hzd/string.hpp"
#include <unistd.h>
#include <filesystem>
#include "exceptions/exceptions.hpp"

namespace detail {
  temp_file::temp_file(const char * contents) {
    std::filesystem::path p = std::filesystem::temp_directory_path() / "j_test_XXXXXX";
    std::string p2(p);
    i32_t fd = mkstemp(p2.data());
    i32_t sz = j::strlen(contents);
    if (fd < 0) {
      throw j::exceptions::system_error_exception() <<
        j::exceptions::message("Could not open a the temporary file.");
    }
    try {
      m_path = p2.c_str();
      auto result = write(fd, contents, sz);
      close(fd);
      if (result < sz) {
      throw j::exceptions::system_error_exception() <<
        j::exceptions::message("Could not write to the temporary file.");
      }
    } catch (...) {
      std::filesystem::remove(p2);
    }
  }

  temp_file::~temp_file() {
    std::filesystem::remove(m_path.as_c_string());
  }
}
