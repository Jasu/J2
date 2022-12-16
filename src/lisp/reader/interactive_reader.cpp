#include "lisp/reader/interactive_reader.hpp"

#include "lisp/reader/build_ast.hpp"
#include "lisp/reader/read.hpp"
#include "lisp/sources/source.hpp"

namespace j::lisp::reader {
  interactive_reader::interactive_reader(env::environment * J_NOT_NULL env,
                                         packages::pkg * J_NOT_NULL pkg,
                                         j::functions::bound_function<void (lisp_imm)> callback)
    : input_buffer(4096),
      state(env, pkg, input_buffer.data_begin),
      callback(callback)
  {
  }

  interactive_read_status interactive_reader::read(sources::string_source * J_NOT_NULL src) {
    strings::const_string_view source_text = src->src;
    read_status status = read_status::waiting;
    while (source_text) {
      u32_t count = input_buffer.write(source_text);
      state.lex_state.limit += count;
      source_text.remove_prefix(count);
      status = read_interactive(*this);
    }
    return status == read_status::waiting && state.is_at_top_level()
      ? interactive_read_status::finished
      : interactive_read_status::pending;
  }

  interactive_read_status interactive_reader::finalize() {
    return (read_interactive(*this) == read_status::waiting && state.is_at_top_level())
      ? interactive_read_status::finished
      : interactive_read_status::pending;
  }

  void interactive_reader::on_top_level() {
    callback(build_form_ast(state, state.begin()));
    state.m_pool.clear();
    state.num_root_elements = 0U;
  }
}
