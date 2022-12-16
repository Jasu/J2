#pragma once

namespace j {
  /// Unmangle null-terminated symbol name.
  ///
  /// The result is zero-terminated and must be released with unmangle_free.
  const char * demangle(const char * mangled_name);

  /// Free a name allocated with unmangle.
  void demangle_free(const char * mangled_name);
}
