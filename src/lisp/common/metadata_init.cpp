#include "lisp/common/metadata_init.hpp"

namespace j::lisp::inline common {
  namespace {
    [[nodiscard]] u32_t value_size(const value_metadata_init & value, const value_metadata * from) noexcept {
      if (!from
          && !value.id
          && !value.name
          && value.name_format == operand_name_format::default_format
          && value.format == format_hint::none) {
        return 0U;
      }
      return sizeof(metadata) + align_up((value.name ? value.name.size() + 1U : ((from && from->name()) ? from->name().size() + 1U : 0U)), 4);
    }

    [[nodiscard]] char * write_str(metadata * J_NOT_NULL md, strings::const_string_view s) noexcept {
      char * to = (char*)(md + 1);
      if (!s) {
        return to;
      }
      u32_t sz = align_up(s.size() + 1U, 4U);
      md->text_size = s.size() + 1U;
      ::j::memzero(to, sz);
      ::j::memcpy(to, s.data(), s.size());
      return to + sz;
    }

    [[nodiscard]] char * write_value(char * J_NOT_NULL to, metadata * & md, i8_t index, const value_metadata_init & value, const value_metadata * from) noexcept {
      if (!from
          && !value.id
          && !value.name
          && value.name_format == operand_name_format::default_format
          && value.format == format_hint::none) {
        return to;
      }
      md = ::new (to) metadata{
        .key = index >= 0 ? metadata_key_op((u8_t)index) : (metadata_key)metadata_key::result,
        .value_metadata{
          .id = value.id ? value.id : ((from && from->id) ? from->id : id{}),
          .name_format = value.name_format != operand_name_format::default_format ? value.name_format : (from ? from->name_format : operand_name_format::default_format),
          .format = value.format != format_hint::none ? value.format : (from ? from->format : format_hint::none),
        },
      };
      return write_str(md, value.name ? value.name : (from ? from->name() : ""));
    }
  }

  [[nodiscard]] u32_t metadata_init::size() const noexcept {
    u32_t sz = 0U;
    if (loc || (src && src->loc())) {
      sz += sizeof(metadata);
    }
    strings::const_string_view c = comment;
    if (!c && src) {
      c = src->comment();
    }
    if (c) {
      sz += sizeof(metadata) + align_up(c.size() + 1U, 4U);
    }
    sz += value_size(result, (src && result.default_from >= -2) ? src->operand(result.default_from) : nullptr);
    i8_t i = 0U;
    for (auto & a : args) {
      i8_t from = a.default_from;
      if (from == -2 && default_value_offset != -2) {
        from = default_value_offset + i;
      }
      sz += value_size(a, (src && from) ? src->operand(from) : nullptr);
      i++;
    }
    return sz;
  }

  void metadata_init::write(char * J_NOT_NULL to) const noexcept {
    sources::source_location l = loc ? loc : (src ? src->loc() : sources::source_location{});
    metadata * md = nullptr;
    if (l) {
      md = ::new (to) metadata{
        .key = metadata_key::loc,
        .source_location = l,
      };
      to += sizeof(metadata);
    }
    strings::const_string_view c = comment;
    if (!c && src) {
      c = src->comment();
    }
    if (c) {
      md = ::new (to) metadata{.key = metadata_key::comment};
      to = write_str(md, c);
    }
    to = write_value(to, md, -1, result, (src && result.default_from >= -2) ? src->operand(result.default_from) : nullptr);
    i8_t i = 0U;
    for (auto & a : args) {
      i8_t from = a.default_from;
      if (from == -2 && default_value_offset != -2) {
        from = default_value_offset + i;
      }
      to = write_value(to, md, i, a, (src && from) ? src->operand(from) : nullptr);
      i++;
    }
    if (md) {
      md->is_last = true;
    }
  }
}
