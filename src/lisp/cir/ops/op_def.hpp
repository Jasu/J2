#pragma once

#include "lisp/cir/ops/input.hpp"
#include "lisp/cir/ops/op.hpp"
#include "lisp/cir/ssa/bb_edge.hpp"
#include "lisp/cir/locs/loc_mask.hpp"
#include "hzd/type_list.hpp"

namespace j::mem {
  class bump_pool;
}

namespace j::lisp::cir::detail {
  enum class operand_kind : u8_t {
    aux = 0U,
    input,
    static_def,
    exit,
  };

  [[nodiscard]] J_RETURNS_NONNULL op * create_op(j::mem::bump_pool & pool, u32_t size, op_type type, bool has_result) noexcept;

  struct input_def_param final {
    J_NO_DEBUG static constexpr operand_kind kind_v = operand_kind::input;
    J_NO_DEBUG static constexpr u32_t index_add_v = 1U;

    J_INLINE_GETTER static const input & read(const op * J_NOT_NULL o, u32_t index) noexcept {
      return reinterpret_cast<const input *>(o + 1)[index];
    }

    template<typename T>
    J_INLINE_GETTER_NO_DEBUG static constexpr u32_t get_size(const T &) noexcept {
      return sizeof(input);
    }

    template<typename T>
    static void write(op * J_NOT_NULL const target, T && value) noexcept {
      (::new (reinterpret_cast<input *>(target + 1) + target->num_inputs) input(static_cast<T &&>(value)))->attach(target->num_inputs++);
    }

    J_ALWAYS_INLINE static void write(op * J_NOT_NULL const target, cir::op * value) noexcept {
      auto input_ptr = reinterpret_cast<input *>(target + 1) + target->num_inputs;
      if (value) {
        (::new (input_ptr) input (value))->attach(target->num_inputs);
      } else {
        ::new (input_ptr) input ();
      }
      target->num_inputs++;
    }
  };

  struct inputs_def final {
    J_NO_DEBUG static constexpr operand_kind kind_v = operand_kind::input;
    J_NO_DEBUG static constexpr u32_t index_add_v = 1U;

    [[nodiscard]] inline static span<const input> read(const op * o, i32_t index) noexcept {
      return { o->inputs_begin() + index, o->num_inputs - index };
    }

    J_INLINE_GETTER static constexpr u32_t get_size(const span<op * const> & defs) noexcept {
      return sizeof(input) * defs.size();
    }
    J_INLINE_GETTER static constexpr u32_t get_size(const span<input const> & defs) noexcept {
      return sizeof(input) * defs.size();
    }

    template<u32_t N>
    J_ALWAYS_INLINE_NO_DEBUG static constexpr u32_t get_size(op * const [N]) noexcept {
      return sizeof(input) * N;
    }

    template<u32_t N>
    J_ALWAYS_INLINE_NO_DEBUG static constexpr u32_t get_size(const input [N]) noexcept {
      return sizeof(input) * N;
    }

    static inline void write(op * J_NOT_NULL const target, const span<op * const> & defs) noexcept {
      auto * in = target->inputs_begin() + target->num_inputs;
      for (auto & def : defs) {
        if (def) {
          (::new (in) input (def))->attach(target->num_inputs);
        } else {
          ::new (in) input ();
        }
        target->num_inputs++;
        ++in;
      }
    }

    static inline void write(op * J_NOT_NULL const target, const span<input const> & defs) noexcept {
      auto * in = target->inputs_begin() + target->num_inputs;
      for (auto & def : defs) {
        (::new (in++) input (def))->attach(target->num_inputs++);
      }
    }

    template<u32_t N>
    J_ALWAYS_INLINE_NO_DEBUG static void write(op * const target, op * const defs[N]) noexcept {
      write(target, span<op *>(defs));
    }

    template<u32_t N>
    J_ALWAYS_INLINE_NO_DEBUG static void write(op * const target, input const defs[N]) noexcept {
      write(target, span<const input>(defs));
    }
  };

  template<typename T>
  struct static_def final {
    J_NO_DEBUG static constexpr u32_t index_add_v = sizeof(T);
    J_NO_DEBUG static constexpr operand_kind kind_v = operand_kind::static_def;

    J_INLINE_GETTER static const T & read(const op * J_NOT_NULL o, u32_t index) noexcept {
      return *add_bytes<const T*>(o->static_begin(), index);
    }

    template<typename U>
    J_INLINE_GETTER_NO_DEBUG static constexpr u32_t get_size(const U &) noexcept {
      return sizeof(T);
    }

    template<typename U>
    inline static u32_t write(op * J_NOT_NULL o, u32_t index, U && value) noexcept {
      ::new (add_bytes(o->static_begin(), index)) T(static_cast<U &&>(value));
      return index + sizeof(T);
    }
  };

  struct aux_reg_def final {
    J_NO_DEBUG static constexpr operand_kind kind_v = operand_kind::aux;
    J_NO_DEBUG static constexpr u32_t index_add_v = 1U;

    J_INLINE_GETTER static loc read(const op * J_NOT_NULL o, u32_t index) noexcept {
      return o->aux_reg(index);
    }
  };

  struct exit_def final {
    J_NO_DEBUG static constexpr operand_kind kind_v = operand_kind::exit;
    J_NO_DEBUG static constexpr u32_t index_add_v = 1U;

    J_INLINE_GETTER_NONNULL static const bb * read(const op * J_NOT_NULL o, u32_t index) noexcept {
      return o->exits_begin()[index].bb;
    }

    J_INLINE_GETTER_NO_DEBUG static constexpr u32_t get_size(bb *) noexcept {
      return sizeof(bb_edge);
    }

    inline static void write(op * J_NOT_NULL target, bb * bb) noexcept {
      ::new (target->exits_begin() + target->num_exits++) bb_edge{bb, 0U};
    }
  };

  template<typename Def, typename T>
  J_A(AI,ND,HIDDEN) inline void maybe_write_static(u32_t, void *, const T &) noexcept { }

  template<typename Def, typename T>
  inline enable_if_t<Def::kind_v == operand_kind::static_def> maybe_write_static(u32_t & index, op * J_NOT_NULL op, T && value) noexcept {
    index = Def::write(op, index, static_cast<T &&>(value));
  }

  template<typename Def, typename T>
  J_A(AI,ND,HIDDEN) inline void maybe_write_input(op *, const T &) noexcept { }

  template<typename Def, typename T>
  inline enable_if_t<Def::kind_v == operand_kind::input> maybe_write_input(op * J_NOT_NULL op, T && value) noexcept {
    Def::write(op, static_cast<T &&>(value));
  }

  template<typename Def, typename T>
  J_A(AI,ND,HIDDEN) inline void maybe_write_exit(void *, const T &) noexcept { }

  template<typename Def, typename T>
  J_A(AI,ND,HIDDEN) inline enable_if_t<is_same_v<Def, exit_def>> maybe_write_exit(op * J_NOT_NULL op, T && b) noexcept {
    ::new (op->exits_begin() + op->num_exits++) bb_edge{(bb*)b, 0U};
  }

  template<typename T> J_HIDDEN constexpr inline u32_t static_def_size_v J_NO_DEBUG = 0U;
  template<typename T> J_HIDDEN constexpr inline u32_t static_def_size_v<static_def<T>> J_NO_DEBUG = sizeof(T);

  template<bool HasResult, typename... ParamDefs>
  struct op_writer;

  template<op_type Type, bool HasResult, typename... ParamDefs>
  struct op_def final {

    using writer_t J_NO_DEBUG_TYPE = op_writer<HasResult, ParamDefs...>;

    J_NO_DEBUG static constexpr inline op_type type_v = Type;

    /// Size of static data of op.
    J_NO_DEBUG static inline constexpr u32_t static_size_v = align_up((0 + ... + static_def_size_v<ParamDefs>), 8U);

    u32_t indices[1 + sizeof...(ParamDefs)] = { 0U };

    explicit consteval op_def() noexcept {
      u32_t param_index = 0U;
      u32_t per_type_indices[4] = {0U};
      ((indices[param_index++] = (per_type_indices[(u8_t)ParamDefs::kind_v] += ParamDefs::index_add_v) - ParamDefs::index_add_v),...);
    }

    template<typename Visitor, bool Enable = !HasResult && sizeof...(ParamDefs)>
    enable_if_t<Enable, decltype(declref<Visitor>()(value_tag<Type>{}, (const op*)nullptr, ParamDefs::read((const op*)nullptr, 0)...))>
    J_NO_DEBUG visit(const op * J_NOT_NULL o, Visitor & visitor) const {
      if constexpr (sizeof...(ParamDefs) == 1) {
        return visitor(value_tag<Type>{}, o, __type_pack_element<0, ParamDefs...>::read(o, 0));
      } else if constexpr (sizeof...(ParamDefs) == 2) {
        return visitor(value_tag<Type>{}, o,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]));
      } else if constexpr (sizeof...(ParamDefs) == 3) {
        return visitor(value_tag<Type>{}, o,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]),
                       __type_pack_element<2, ParamDefs...>::read(o, indices[2]));
      } else if constexpr (sizeof...(ParamDefs) == 4) {
        return visitor(value_tag<Type>{}, o,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]),
                       __type_pack_element<2, ParamDefs...>::read(o, indices[2]),
                       __type_pack_element<3, ParamDefs...>::read(o, indices[3]));
      } else {
        static_assert(sizeof...(ParamDefs) == 5);
        return visitor(value_tag<Type>{}, o,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]),
                       __type_pack_element<2, ParamDefs...>::read(o, indices[2]),
                       __type_pack_element<3, ParamDefs...>::read(o, indices[3]),
                       __type_pack_element<4, ParamDefs...>::read(o, indices[4]));
      }
    }

    template<typename Visitor, bool Enable = HasResult && sizeof...(ParamDefs)>
    enable_if_t<Enable, decltype(declref<Visitor>()(value_tag<Type>{}, declref<const op *>(), declref<const cir::loc>(), ParamDefs::read((const op*)nullptr, 0)...))>
    J_NO_DEBUG visit(const op * J_NOT_NULL o, Visitor & visitor) const {
      if constexpr (sizeof...(ParamDefs) == 1) {
        return visitor(value_tag<Type>{}, o, o->result.loc_out, __type_pack_element<0, ParamDefs...>::read(o, 0));
      } else if constexpr (sizeof...(ParamDefs) == 2) {
        return visitor(value_tag<Type>{}, o,
                       o->result.loc_out,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]));
      } else {
        static_assert(sizeof...(ParamDefs) == 3);
        return visitor(value_tag<Type>{}, o,
                       o->result.loc_out,
                       __type_pack_element<0, ParamDefs...>::read(o, 0),
                       __type_pack_element<1, ParamDefs...>::read(o, indices[1]),
                       __type_pack_element<2, ParamDefs...>::read(o, indices[2]));
      }
    }

    template<typename Visitor>
    J_NO_DEBUG decltype(auto) visit(const void * J_NOT_NULL o, Visitor & visitor) const {
      return visitor(value_tag<Type>{}, reinterpret_cast<const op *>(o));
    }

    template<typename Visitor, bool Enable = HasResult>
    J_NO_DEBUG
    enable_if_t<HasResult, decltype(declref<Visitor>()(value_tag<Type>{}, (const op*)nullptr, declref<const cir::loc>()))>
    visit(const conditional_t<sizeof...(ParamDefs) != 0, void, op> * J_NOT_NULL o, Visitor & visitor) const {
      return visitor(value_tag<Type>{}, reinterpret_cast<const op *>(o), reinterpret_cast<const op *>(o)->result.loc_out);
    }
  };

  template<op_type Type>
  struct op_def<Type, true, input_def_param, input_def_param> final {
    using writer_t J_NO_DEBUG_TYPE = op_writer<true, input_def_param, input_def_param>;
    J_NO_DEBUG static constexpr inline op_type type_v = Type;

    /// Size of static data of op.
    J_NO_DEBUG static inline constexpr u32_t static_size_v = 0U;

    template<typename Visitor>
    J_NO_DEBUG
    decltype(declref<Visitor>()(value_tag<Type>{}, declref<const op *>(), declref<const cir::loc>(), declref<const input>(), declref<const input>()))
    visit(const op * J_NOT_NULL o, Visitor & visitor) const {
      const auto inputs = reinterpret_cast<const input *>(o + 1);
      return visitor(value_tag<Type>{}, o, o->result.loc_out, inputs[0], inputs[1]);
    }

    template<typename Visitor>
    J_ALWAYS_INLINE_NO_DEBUG decltype(auto) visit(const void * J_NOT_NULL o, Visitor & visitor) const {
      return visitor(value_tag<Type>{}, reinterpret_cast<const op *>(o));
    }

    template<typename Visitor>
    J_ALWAYS_INLINE_NO_DEBUG decltype(declref<Visitor>()(value_tag<Type>{}, (const op*)nullptr, declref<const cir::loc>()))
    visit(void * J_NOT_NULL o, Visitor & visitor) const {
      return visitor(value_tag<Type>{}, reinterpret_cast<const op *>(o), reinterpret_cast<const op *>(o)->result.loc_out);
    }
  };


  template<bool HasResult, typename... ParamDefs>
  struct op_writer final {
    template<typename... Args>
    static J_NO_DEBUG J_RETURNS_NONNULL op * write(j::mem::bump_pool & pool, u32_t pad, op_type t, Args && ... params) noexcept {
      op * const result = create_op(pool, align_up((ParamDefs::get_size(params) + ... + sizeof(op)), 8U) + pad, t, HasResult);
      (maybe_write_input<ParamDefs>(result, static_cast<Args &&>(params)), ...);

      (maybe_write_exit<ParamDefs>(result, static_cast<Args &&>(params)), ...);

      u32_t index = 0U;
      (maybe_write_static<ParamDefs>(index, result, static_cast<Args &&>(params)), ...);

      return result;
    }
  };

  template<>
  struct op_writer<true, detail::input_def_param, detail::input_def_param> final {
    template<typename In1, typename In2>
    static J_NO_DEBUG J_RETURNS_NONNULL op * write(j::mem::bump_pool & pool, u32_t pad, op_type t, In1 && in1, In2 && in2) noexcept {
      op * const result = create_op(pool, align_up(sizeof(op) + 2 * sizeof(input), 8U) + pad, t, true);
      input_def_param::write(result, static_cast<In1 &&>(in1));
      input_def_param::write(result, static_cast<In2 &&>(in2));
      return result;
    }
  };

  template<>
  struct op_writer<false, detail::input_def_param, detail::static_def<imm_type_mask>, detail::aux_reg_def, detail::exit_def, detail::exit_def> final {
    template<typename In>
    static J_NO_DEBUG J_RETURNS_NONNULL op * write(j::mem::bump_pool & pool,
                                        u32_t pad,
                                        op_type t,
                                        In && input,
                                        const imm_type_mask & mask,
                                        bb * exit0,
                                        bb * exit1) noexcept {

      op * const result = create_op(pool,
                                    align_up(sizeof(op) + sizeof(input) +align_up(sizeof(imm_type_mask), 8U) + 2 * sizeof(bb_edge) + align_up(sizeof(loc), 8U), 8U) + pad,
                                    t, false);
      result->max_aux_regs = 1U;
      result->num_aux_regs = mask == imm_i64 ? 0U : 1U;

      input_def_param::write(result, static_cast<In &&>(input));
      ::new ((loc*)result->inputs_end()) loc{};

      auto exits = result->exits_begin();
      ::new (exits) bb_edge{exit0, 0U};
      ::new (exits + 1) bb_edge{exit1, 0U};
      result->num_exits = 2U;

      detail::static_def<imm_type_mask>::write(result, 0U, mask);

      return result;
    }
  };
}
