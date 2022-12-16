#include "lisp/mem/code_heap/code_heap.hpp"
#include "mem/vmem/vmem.hpp"
#include "hzd/mem.hpp"

namespace j::lisp::mem::code_heap {
  code_heap::code_heap(u32_t code_size, u32_t data_size)
  {
    code_size = page_align_up(code_size);
    data_size = page_align_up(data_size);
    heap = j::mem::vmem::map(code_size + data_size, j::mem::vmem::alloc_flags::no_reserve);
    auto data_heap = add_bytes(heap, code_size);
    j::mem::vmem::protect(heap, code_size, j::mem::vmem::protection_flags::rx);
    j::mem::vmem::protect(data_heap, data_size, j::mem::vmem::protection_flags::r);
    code.initialize(heap, code_size);
    data.initialize(data_heap, data_size);
  }

  code_heap::~code_heap() {
    j::mem::vmem::unmap(heap, code.capacity + data.capacity);
  }

  allocation code_heap::allocate(u32_t code_size, u32_t data_size) {
    J_ASSUME(code_size > 0U);
    if (data_size) {
      return allocation(this, code.allocate(code_size), data.allocate(data_size));
    }
    return allocation(this, code.allocate(code_size), nullptr);
  }

  namespace {
    void assign_region(subheap_allocation & alloc, j::mem::const_memory_region buf, j::mem::vmem::protection_flags prot) {
      J_ASSERT_NOT_NULL(alloc, buf);
      j::mem::memory_region reg = alloc.region();
      J_ASSERT(buf.size() <= reg.size());
      void * pages_begin = j::page_align_down(reg.begin());
      void * pages_end = j::page_align_up(reg.end());
      iptr_t full_size = byte_offset(pages_begin, pages_end);
      iptr_t prefix_length = byte_offset(pages_begin, reg.begin());
      iptr_t suffix_length = byte_offset(reg.end(), pages_end);
      J_ASSERT(full_size >= buf.size());
      void *new_pages = j::mem::vmem::map(full_size);
      void *new_pages_end = add_bytes(new_pages, J_PAGE_SIZE);
      if (prefix_length) {
        ::j::memcpy(new_pages, pages_begin, prefix_length);
      }
      if (suffix_length) {
        ::j::memcpy(subtract_bytes(new_pages_end, suffix_length), subtract_bytes(pages_end, suffix_length), suffix_length);
      }
      buf.copy_to(add_bytes(new_pages, prefix_length));
      j::mem::vmem::protect(new_pages, full_size, prot);
      j::mem::vmem::replace_pages(pages_begin, new_pages, full_size);
    }
  }
  void allocation::assign_code(j::mem::const_memory_region code) {
    assign_region(code_allocation, code, j::mem::vmem::protection_flags::rx);
  }

  void allocation::assign_data(j::mem::const_memory_region data) {
    assign_region(data_allocation, data, j::mem::vmem::protection_flags::r);
  }

  void allocation::release() noexcept {
    if (heap) {
      code_allocation.release(&heap->code);
      if (data_allocation) {
        data_allocation.release(&heap->data);
      }
      heap = nullptr;
    }
  }
}
