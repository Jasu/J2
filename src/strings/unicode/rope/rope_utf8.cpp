#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/unicode/rope/rope_debug.hpp"
#include "strings/ropes/rope_string.hpp"
#include "containers/trees/rope_tree.hpp"
#include "strings/unicode/rope/rope_utf8_byte_iterator.hpp"

J_DEFINE_EXTERN_ROPE_TREE(j::strings::rope_utf8_description);

namespace j::strings::inline unicode::inline rope {
  namespace {
    inline rope_utf8_byte_iterator to_byte_iterator(rope_utf8_chunk_iterator it, u16_t index = 0U) noexcept {
      char *begin = nullptr, *end = nullptr;
      if (it.is_at_end_fuck_sartre()) {
        --it;
        begin = end = it->text.end();
        J_ASSUME_NOT_NULL(begin);
      } else {
        const auto text = it->text;
        begin = text.begin() + index;
        end = text.end();
      }
      return rope_utf8_byte_iterator{begin, end, it};
    }

    struct J_TYPE_HIDDEN truncated_code_point final {
      u8_t size = 0U;
      u8_t truncated_by = 0U;
      char buffer[3U] = { '\0' };

      explicit operator bool() const noexcept {
        J_ASSUME(size <= 3);
        return size;
      }

      void prepend(char byte) {
        J_ASSUME(size < 3);
        buffer[2U] = buffer[1U];
        buffer[1U] = buffer[0U];
        buffer[0U] = byte;
        ++size;
      }
    };

    inline truncated_code_point truncated_code_point_before(const_rope_utf8_byte_iterator it) {
      truncated_code_point result;
      for (u8_t i = 0U, sz = ::j::min(3U, it.position()); i < sz; ++i) {
        --it;
        if (utf8_is_successor_byte(*it)) {
          result.prepend(*it);
          continue;
        }
        const u8_t truncated_by = utf8_leading_byte_code_point_bytes(*it) - 1U;
        if (truncated_by > i) {
          result.truncated_by = truncated_by - i;
          result.prepend(*it);
          return result;
        }
        break;
      }
      return {};
    }

    struct J_TYPE_HIDDEN stolen_successor_bytes final {
      u8_t size = 0U;
      char buffer[3U] = { '\0' };

      explicit operator bool() const noexcept {
        J_ASSUME(size <= 3);
        return size;
      }

      void append(char byte) noexcept {
        J_ASSUME(size < 3);
        buffer[size++] = byte;
      }
    };

    stolen_successor_bytes get_stolen_successor_bytes(const_rope_utf8_byte_iterator it, u8_t truncated_by) {
      stolen_successor_bytes result{};
      J_ASSUME(truncated_by < 4);
      for (u8_t i = 0; i < truncated_by && it && utf8_is_successor_byte(*it); ++i, ++it) {
        result.append(*it);
      }
      return result;
    }

    inline stolen_successor_bytes count_take_after(const_rope_utf8_byte_iterator it, const_string_view str) {
      if (!it || J_LIKELY(!utf8_is_successor_byte(*it))) {
        return {};
      }
      u8_t truncated_by = 0U;
      const u32_t sz = str.size();
      for (u8_t i = 0, cnt = ::j::min(3U, sz); i < cnt; ++i) {
        if (!utf8_is_successor_byte(str[sz - 1U - i])) {
          truncated_by = utf8_leading_byte_code_point_bytes(str[sz - 1U - i]) - 1U;
          if (truncated_by > i) {
            truncated_by -= i;
          } else {
            truncated_by = 0U;
          }
          break;
        }
      }
      return get_stolen_successor_bytes(it, truncated_by);
    }

    rope_utf8_byte_iterator handle_range_erase(rope_utf8_tree & tree,
                                               const_rope_utf8_byte_iterator start,
                                               const_rope_utf8_byte_iterator end)
    {
      u64_t start_pos = start.position(), end_pos = end.position();
      J_ASSERT(start_pos <= end_pos, "Unordered iterators");
      if (!end.is_at_end() && utf8_is_successor_byte(*end)
          && !start.is_at_begin())
      {
        const auto trunc = truncated_code_point_before(start);
        if (const auto stolen = get_stolen_successor_bytes(end, trunc.truncated_by)) {
          start_pos -= trunc.size;
          end_pos += stolen.size;
          auto it = tree.erase(start_pos, end_pos - start_pos);
          i32_t sz = trunc.size + stolen.size;
          rope_string * rope_str = rope_string::allocate(sz);
          char * buf = rope_str->data();
          ::j::memcpy(buf, trunc.buffer, trunc.size);
          buf += trunc.size;
          ::j::memcpy(buf, stolen.buffer, stolen.size);
          it = tree.emplace(it, string_view{rope_str->data(), sz}, rope_str);
          return { buf, buf + stolen.size, it };
        }
      }
      return to_byte_iterator(tree.erase(start_pos, end_pos - start_pos));
    }
  }

  rope_utf8_byte_iterator rope_utf8::insert(
    const_rope_utf8_byte_iterator it_,
    const_string_view str
  ) {
    rope_utf8_byte_iterator it = reinterpret_cast<rope_utf8_byte_iterator &>(it_);
    auto take_before = utf8_is_successor_byte(str[0])
      ? truncated_code_point_before(it)
      : truncated_code_point{};
    auto take_after = count_take_after(it, str);

    const i32_t size_before = take_before.size;
    const i32_t size_after = take_after.size;
    J_ASSUME(size_before < 4);
    J_ASSUME(size_after < 4);

    if (it.is_at_chunk_end()) {
      auto prev = it.m_char == it.m_end ? it.m_it : it.m_it.previous();
      J_ASSERT_NOT_NULL(prev);
      if (prev->space_after() >= (u32_t)(str.size() + size_after)) {
        if (size_after) {
          // J_DEBUG("Insert chunk end 1");
          erase(it, size_after);
          prev->append_in_place(m_tree.new_modification_stamp(), str, { take_after.buffer, size_after });
          validate_iterator({ prev->text.end() - size_after, prev->text.end(), prev });
          return { prev->text.end() - size_after, prev->text.end(), prev };
        } else {
          // J_DEBUG("Insert chunk end 2");
          prev->append_in_place(m_tree.new_modification_stamp(), str);
          if (prev == it.m_it) {
            it.m_end = prev->text.end();
          }
          validate_iterator(it);
          return reinterpret_cast<rope_utf8_byte_iterator &>(it);
        }
      }
    }

    if (it.is_at_chunk_begin() && it.m_it->space_before() >= (u32_t)(str.size() + size_before)) {
      if (size_before) {
        // J_DEBUG("Insert chunk begin 1");
        erase(it - size_before, size_before);
        it.m_it->prepend_in_place(m_tree.new_modification_stamp(), { take_before.buffer, size_before }, str);
        validate_iterator({ it.m_it->text.begin() + size_before, it.m_it->text.end(), it.m_it });
        return { it.m_it->text.begin() + size_before, it.m_it->text.end(), it.m_it };
      } else {
        // J_DEBUG("Insert chunk begin 2");
        it.m_it->prepend_in_place(m_tree.new_modification_stamp(), str);
        validate_iterator(it);
        return it;
      }
    }

    const i32_t total_sz = str.size() + size_before + size_after;
    J_ASSUME(total_sz > 0);
    rope_string * const rope_str = rope_string::allocate(total_sz);
    char * write_ptr = rope_str->data();


    if (size_before) {
      it -= size_before;
      ::j::memcpy(write_ptr, take_before.buffer, size_before);
      write_ptr +=size_before;
    }
    ::j::memcpy(write_ptr, str.data(), str.size());
    write_ptr += str.size();
    if (size_after) {
      ::j::memcpy(write_ptr, take_after.buffer, size_after);
    }
    if (size_before || size_after) {
      it = erase(it, size_before + size_after);
    }

    const string_view sv{rope_str->data(), total_sz};
    auto chunk_it = m_tree.emplace_at(it.m_it,
                                      it.m_char - it.m_it->text.begin(),
                                      sv, rope_str);
    // J_DEBUG("Last insert case", sv.begin() + size_before + str.size() == sv.end(), chunk_it.has_next());
    validate_iterator({ sv.begin() + size_before, sv.end(), chunk_it });
    if (sv.begin() + size_before + str.size() == sv.end() && chunk_it.has_next()) {
      chunk_it++;
      validate_iterator({ chunk_it->text.begin(), chunk_it->text.end(), chunk_it });
      return { chunk_it->text.begin(), chunk_it->text.end(), chunk_it };
    }
    return { sv.begin() + size_before, sv.end(), chunk_it };
  }

  void rope_utf8::prepend(const_string_view str) {
    if (J_LIKELY(str)) {
      insert(bytes().begin(), str);
    }
  }

  void rope_utf8::append(const_string_view str) {
    if (J_LIKELY(str)) {
      insert(bytes().end(), str);
    }
  }

  void rope_utf8::erase(u64_t at, u64_t size) {
    handle_range_erase(m_tree, bytes().iterate_at(at), bytes().iterate_at(at + size));
  }

  rope_utf8_byte_iterator rope_utf8::erase(const const_rope_utf8_byte_iterator at, u64_t size) {
    return handle_range_erase(m_tree, at, bytes().iterate_at(at.position() + size));
  }

  rope_utf8_byte_iterator rope_utf8::erase(const const_rope_utf8_byte_iterator start,
                                           const const_rope_utf8_byte_iterator end)
  {
    return handle_range_erase(m_tree, start, end);
  }

  rope_utf8_code_point_iterator rope_utf8::erase(const rope_utf8_code_point_iterator at, u64_t size) {
    return rope_utf8_code_point_iterator(erase((const_rope_utf8_byte_iterator)at, size));
  }

  rope_utf8_code_point_iterator rope_utf8::erase(const rope_utf8_code_point_iterator start,
                                                 const rope_utf8_code_point_iterator end)
  {
    return rope_utf8_code_point_iterator(erase((const_rope_utf8_byte_iterator)start, (const_rope_utf8_byte_iterator)end));
  }


  rope_utf8_byte_iterator rope_utf8::split(const_rope_utf8_byte_iterator at) {
    const rope_utf8_byte_iterator & it = reinterpret_cast<rope_utf8_byte_iterator &>(at);
    return to_byte_iterator(m_tree.split(it.position()));
  }

  rope_utf8_byte_iterator rope_utf8::split(u64_t at) {
    return to_byte_iterator(m_tree.split(at));
  }

  rope_utf8_byte_iterator rope_utf8::find_line(u64_t line_number) {
    auto p = m_tree.find_by_secondary(hard_line_breaks_key(line_number));
    return to_byte_iterator(p.first, p.second);
  }

  rope_utf8_view rope_utf8::view_at(const_rope_utf8_byte_iterator begin,
                                    const_rope_utf8_byte_iterator end) noexcept
  {
    return {
      reinterpret_cast<const rope_utf8_byte_iterator &>(begin),
      reinterpret_cast<const rope_utf8_byte_iterator &>(end)
    };
  }

  const const_rope_utf8_view rope_utf8::view_at(const_rope_utf8_byte_iterator begin,
                                                const_rope_utf8_byte_iterator end) const noexcept {
    return { begin, end };
  }

  rope_utf8::operator rope_utf8_view() noexcept {
    return { bytes().begin(), bytes().end() };
  }

  rope_utf8::operator const_rope_utf8_view() const noexcept {
    return { bytes().begin(), bytes().end() };
  }

  template struct basic_rope_utf8_byte_iterator<false>;
  template struct basic_rope_utf8_byte_iterator<true>;

  template i64_t operator-<true>(
    const basic_rope_utf8_byte_iterator<true> & lhs,
    const basic_rope_utf8_byte_iterator<true> & rhs) noexcept;
  template i64_t operator-<false>(
    const basic_rope_utf8_byte_iterator<false> & lhs,
    const basic_rope_utf8_byte_iterator<false> & rhs) noexcept;
}
