#include "lisp/assembly/linking/reloc.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::lisp::assembly::linking::reloc);

namespace j::lisp::assembly::inline linking {
  namespace {
    void set_field_value(reloc_field_options opts, void * J_NOT_NULL write_field, i64_t value) noexcept {
      switch (opts.type) {
      case reloc_type::rel32: {
        const i32_t truncated = value;
        J_ASSERT(truncated == value);
        J_MEMCPY_INLINE(write_field, &truncated, 4U);
        break;
      }
      case reloc_type::rel16: {
        const i16_t truncated = value;
        J_ASSERT(truncated == value);
        J_MEMCPY_INLINE(write_field, &truncated, 2U);
        break;
      }
      case reloc_type::rel8: {
        const i8_t truncated = value;
        J_ASSERT(truncated == value);
        J_MEMCPY_INLINE(write_field, &truncated, 1U);
        break;
      }
      case reloc_type::abs64:
        J_MEMCPY_INLINE(write_field, &value, 8U);
        return;
      case reloc_type::none:
        J_ASSERT_FAIL("Relocation type was none.");
      }
    }
    i64_t get_field_value(reloc_field_options opts, void * J_NOT_NULL write_field) noexcept {
      switch (opts.type) {
      case reloc_type::abs64:
        return *(i64_t*)write_field;
      case reloc_type::rel32:
        return *(i32_t*)write_field;
      case reloc_type::rel16:
        return *(i16_t*)write_field;
      case reloc_type::rel8:
        return *(i8_t*)write_field;
      case reloc_type::none:
        J_UNREACHABLE();
      }
    }
  }
  void apply_reloc(
    void * J_NOT_NULL write_field,
    const void * J_NOT_NULL final_field_pos,
    reloc_field_options opts,
    const void * value) noexcept {
    J_ASSUME(opts.type != reloc_type::none);

    i64_t final_value = reinterpret_cast<i64_t>(value);

    if (opts.type != reloc_type::abs64) {
      // Relative
      final_value -= reinterpret_cast<i64_t>(final_field_pos);
    }

    if (opts.shift < 0) {
      final_value >>= -opts.shift;
    } else if (J_UNLIKELY(opts.shift > 0)) {
      final_value <<= -opts.shift;
    }
    final_value += get_field_value(opts, write_field);
    set_field_value(opts, write_field, final_value);
  }

  void apply_reloc(void * J_NOT_NULL write_field,
                   reloc_field_options opts,
                   const void * value) noexcept {
    apply_reloc(write_field, write_field, opts, value);
  }

  void apply_reloc_rel(
    void * J_NOT_NULL write_field,
    reloc_field_options opts,
    i64_t offset) noexcept {
    J_ASSUME(opts.type != reloc_type::none);
    J_ASSUME(opts.type != reloc_type::abs64);

    if (opts.shift < 0) {
      offset >>= -opts.shift;
    } else if (J_UNLIKELY(opts.shift > 0)) {
      offset <<= -opts.shift;
    }
    offset += get_field_value(opts, write_field);
    set_field_value(opts, write_field, offset);
  }
}
