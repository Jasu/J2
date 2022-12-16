#include "lisp/common/metadata.hpp"
#include "logging/global.hpp"

namespace j::lisp::inline common {
  [[nodiscard]] strings::const_string_view metadata::text() const noexcept {
    return text_size ? strings::const_string_view{(const char *)(this + 1), text_size - 1} : "";
  }

  const metadata * metadata::next() const noexcept {
    return is_last ? nullptr : add_bytes(this, size());
  }

  [[nodiscard]] const metadata * metadata_view::find(metadata_key key) const noexcept {
    if (first) {
      i32_t count = 0;
      for (auto & di : *this) {
        if (di.key == key) {
          return &di;
        }
        ++count;
      }
      J_WARNING_IF(count > 10, "Too much metadata ({} entries)", count);
    }
    return nullptr;
  }

  [[nodiscard]] const value_metadata * metadata_view::result() const noexcept {
    auto ptr = find(metadata_type::result);
    return ptr ? & ptr->value_metadata : nullptr;
  }

  [[nodiscard]] id metadata_view::result_id() const noexcept {
    auto md = result();
    return md ? md->id : id{};
  }

  [[nodiscard]] strings::const_string_view metadata_view::result_name() const noexcept {
    auto md = result();
    return md ? md->name() : "";
  }

  [[nodiscard]] const value_metadata * metadata_view::operand(u8_t index) const noexcept {
    auto ptr = find(metadata_key(metadata_type::operand, index));
    return ptr ? & ptr->value_metadata : nullptr;
  }

  [[nodiscard]] id metadata_view::operand_id(u8_t index) const noexcept {
    auto md = operand(index);
    return md ? md->id : id{};
  }

  [[nodiscard]] strings::const_string_view metadata_view::operand_name(u8_t index) const noexcept {
    auto md = operand(index);
    return md ? md->name() : "";
  }


  [[nodiscard]] strings::const_string_view value_metadata::name() const noexcept {
    return J_CONTAINER_OF(this, metadata, value_metadata)->text();
  }

  [[nodiscard]] operand_name value_metadata::operand_name() const noexcept {
    return {
      .id = id,
      .index = index,
      .name_format = name_format,
      .name = name(),
    };
  }

  [[nodiscard]] strings::const_string_view comment_metadata::comment() const noexcept {
    return J_CONTAINER_OF(this, metadata, comment_metadata)->text();
  }
  [[nodiscard]] sources::source_location metadata_view::loc() const noexcept {
    auto md = find(metadata_key::loc);
    if (md) {
      return md->source_location;
    }
    return {};
  }
  [[nodiscard]] strings::const_string_view metadata_view::comment() const noexcept {
    auto md = find(metadata_key::comment);
    if (md) {
      return md->text();
    }
    return {};
  }
}
