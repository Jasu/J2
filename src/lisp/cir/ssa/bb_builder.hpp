#pragma once

#include "lisp/common/metadata_init.hpp"
#include "lisp/cir/ops/op_type.hpp"
#include "lisp/cir/ops/op.hpp"

namespace j::mem {
  class bump_pool;
}

namespace j::lisp::cir::inline ops {
  struct op;
  struct input;
}
namespace j::lisp::cir::inline ssa {
  struct bb_builder final {
    struct bb * bb = nullptr;
    j::mem::bump_pool * pool = nullptr;

    /// Remove `op` from the basic block.
    J_A(AI,ND,HIDDEN) inline void erase(struct op * J_NOT_NULL op) noexcept {
      erase_no_detach(op);
      detach_op(op);
    }
    void erase_no_detach(struct op * J_NOT_NULL op) noexcept;

    /// Detach `op` from its basic block.
    void detach_op(struct op * J_NOT_NULL op) noexcept;

    void connect_back(op * J_NOT_NULL result) const noexcept;

    template<typename OpDef, typename... Args>
    J_A(AI,ND,HIDDEN,RNN) inline op * emplace_back(const OpDef &, Args && ... args) {
      op * const result = do_emplace<typename OpDef::writer_t>(OpDef::type_v, static_cast<Args &&>(args)...);
      connect_back(result);
      return result;
    }

    void connect_front(op * J_NOT_NULL result) const noexcept;

    template<typename OpDef, typename... Args>
    J_A(AI,ND,HIDDEN,RNN) inline op * emplace_front(const OpDef &, Args && ... args) {
      op * const result = do_emplace<typename OpDef::writer_t>(OpDef::type_v, static_cast<Args &&>(args)...);
      connect_front(result);
      return result;
    }

    void connect_before(op * J_NOT_NULL succ, op * J_NOT_NULL result) const noexcept;

    template<typename OpDef, typename... Args>
    J_A(AI,ND,HIDDEN,RNN) inline op * emplace_before(op * J_NOT_NULL succ, const OpDef &, Args && ... args) {
      op * const result = do_emplace<typename OpDef::writer_t>(OpDef::type_v, static_cast<Args &&>(args)...);
      connect_before(succ, result);
      return result;
    }

    void connect_after(op * pred, op * J_NOT_NULL result) const noexcept;

    template<typename OpDef, typename... Args>
    J_A(AI,ND,HIDDEN,RNN) inline op * emplace_after(op * pred, const OpDef &, Args && ... args) {
      op * const result = do_emplace<typename OpDef::writer_t>(OpDef::type_v, static_cast<Args &&>(args)...);
      connect_after(pred, result);
      return result;
    }

    template<typename OpDef, typename... Args>
    J_A(AI,ND,HIDDEN,RNN) inline op * replace(op * J_NOT_NULL target, const OpDef &, Args && ... args) {
      op * const new_node = do_emplace<typename OpDef::writer_t>(OpDef::type_v, static_cast<Args &&>(args)...);
      replace_with(target, new_node);
      return new_node;
    }

    void replace_with(op * J_NOT_NULL old_node, op * J_NOT_NULL new_node) noexcept;

    void replace_result(op * J_NOT_NULL target, op * J_NOT_NULL source) noexcept;
    void replace_result(op * J_NOT_NULL target, input source) noexcept;
  private:
    void initialize_edges(op * J_NOT_NULL terminal) const noexcept;

    template<typename OpWr, typename... Args>
    J_NO_DEBUG J_RETURNS_NONNULL op * do_emplace(op_type type, Args && ... args) const {
      return OpWr::write(*pool, 0U, type, static_cast<Args &&>(args)...);
    }

    template<typename OpWr, typename... Args>
    J_NO_DEBUG J_RETURNS_NONNULL op * do_emplace(op_type type, metadata_init md, Args && ... args) const {
      u32_t sz = md.size();
      op * result = OpWr::write(*pool, sz, type, static_cast<Args &&>(args)...);
      if (sz) {
        write_op_metadata(result, md);
      }
      return result;
    }
  };
}
