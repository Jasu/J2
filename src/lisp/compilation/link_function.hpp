#pragma once

namespace j::lisp::cir_to_mc {
  struct mc_functions;
}
namespace j::lisp::functions {
  struct func_info;
}
namespace j::lisp::compilation {
  struct context;

  /// Allocates memory for the function.
  void allocate_function(context & ctx, functions::func_info * info, const cir_to_mc::mc_functions * mc_functions);

  /// Link the function to its allocation.
  void link_function(context & ctx, functions::func_info * info, const cir_to_mc::mc_functions * mc_functions);
}
