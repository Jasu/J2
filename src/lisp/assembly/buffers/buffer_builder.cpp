#include "buffer_builder.hpp"

namespace j::lisp::assembly::inline buffers {
  backpatch_record::backpatch_record(
    void * J_NOT_NULL write_field,
    reloc_field_options opts,
    u8_t metadata_size,
    i32_t offset,
    backpatch_record * next) noexcept
    : write_field(write_field),
      opts(opts),
      metadata_size(metadata_size),
      offset(offset),
      next(next)
  {
    J_ASSUME(offset >= 0);
    J_ASSUME(opts.type != reloc_type::none);
    J_ASSUME(opts.type != reloc_type::abs64);
  }

  [[nodiscard]] buffer buffer_builder::build() {
    trivial_array<reloc> relocs(containers::uninitialized, m_relocs.size());
    for (auto & reloc : m_relocs) {
      relocs.initialize_element(reloc);
    }
    m_relocs.clear();
    buffer result{m_builder.build(), static_cast<trivial_array<reloc> &&>(relocs)};
    m_builder.clear();
    return result;
  }

  [[nodiscard]] J_RETURNS_NONNULL label_record * buffer_builder::allocate_label(const strings::const_string_view & label) {
    label_record * rec = &temp_pool->emplace_with_padding<label_record>(label.size());
    rec->label_size = label.size();
    if (label) {
      ::j::memcpy(rec + 1, label.begin(), label.size());
    }
    return rec;
  }

  void buffer_builder::push_label(label_record * J_NOT_NULL label) {
    J_ASSERT(label->is_valid == false && label->offset == 0);
    label->is_valid = true;
    label->offset = m_builder.size();
    for (auto bp = label->first_backpatch; bp; bp = bp->next) {
      apply_reloc_rel(bp->write_field, bp->opts, (i32_t)label->offset - bp->offset);
    }
  }

  void * buffer_builder::append_backpatch_rel(label_record * J_NOT_NULL label, reloc_field_options opts, i64_t addend, u8_t metadata_size) {
    i32_t offset = m_builder.size();
    auto field = allocate((u8_t)opts.type);
    ::j::memcpy(field, &addend, (u8_t)opts.type);
    if (label->is_valid) {
      apply_reloc_rel(field, opts, (i32_t)label->offset - offset);
      return nullptr;
    } else {
      auto bp = &temp_pool->emplace_with_padding<backpatch_record>(metadata_size, field, opts, metadata_size, offset, label->first_backpatch);
      label->first_backpatch = bp;
      return bp + 1;
    }
  }

  void buffer_builder::append_reloc(reloc_source source, reloc_field_options opts, i64_t addend) {
    auto field = allocate((u8_t)opts.type);
    ::j::memcpy(field, &addend, (u8_t)opts.type);
    J_ASSERT(source.source_type != reloc_source_type::none);
    add_reloc(field, source, opts);
  }

  void buffer_builder::write_reloc(void * J_NOT_NULL addr, reloc_source source, reloc_field_options opts, i64_t addend) {
    ::j::memcpy(addr, &addend, (u8_t)opts.type);
    add_reloc(addr, source, opts);
  }

  void buffer_builder::add_reloc(void * J_NOT_NULL addr, reloc_source source, reloc_field_options opts) {
    u32_t offset = m_builder.index_of(addr);
    add_reloc(offset, source, opts);
  }

  void buffer_builder::add_reloc(u32_t offset, reloc_source source, reloc_field_options opts) {
    J_ASSERT(source.source_type != reloc_source_type::none);
    m_relocs.emplace_back(reloc{
      .offset = offset,
      .source_type = source.source_type,
      .cc = source.cc,
      .opts = opts,
      .source_id = source.source_id,
    });
  }

  void buffer_builder::add_reloc(reloc r, u32_t base) {
    J_ASSERT(r.source_type != reloc_source_type::none);
    m_relocs.emplace_back(r).set_base(base);
  }

  u32_t buffer_builder::append_buffer(const buffer & buf) noexcept {
    void * addr = m_builder.append(buf.data);
    u32_t offset = m_builder.index_of(addr);
    for (auto & r : buf.relocs) {
      m_relocs.emplace_back(r).set_base(offset);
    }
    return offset;
  }
}
