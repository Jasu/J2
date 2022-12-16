#pragma once

namespace j::files::inline paths {
  class path;
}

namespace j::properties::wrappers {
  class stored_wrapper;
}

namespace j::serialization {
  class backend {
  public:
    /// Checks whether the backend supports the file (e.g. based on extension).
    ///
    /// The check should not be taken to mean that the serialization or deserialization will
    /// succeed.
    virtual bool supports(const files::path & path) const = 0;

    /// Read the contents of the file as a resource.
    virtual properties::wrappers::stored_wrapper read(const files::path & path) const = 0;

    virtual ~backend();
  };
}
