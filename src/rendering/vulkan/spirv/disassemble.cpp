#include "rendering/vulkan/spirv/disassemble.hpp"
#include "rendering/vulkan/spirv/instruction_iterator.hpp"
#include "rendering/vulkan/spirv/header.hpp"
#include "logging/global.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::vulkan::inline spirv {
  void disassemble(const u8_t * spirv, u32_t size) {
    J_ASSERT_NOT_NULL(spirv, size);
    J_ASSERT((size & 3) == 0, "Size not aligned.");
    J_ASSERT(size > sizeof(spirv_header), "Size does not accommodate header.");
    J_INFO("SPIR-V file of {} bytes", size);
    const spirv_header * header = reinterpret_cast<const spirv_header*>(spirv);
    J_INFO("       Magic: {:08x}   Version: {:08x}", header->magic, header->version);
    J_INFO("  Gen. Magic: {:08x}  ID bound: {}\n", header->generator_magic, header->id_bound);
    instruction_iterator it{spirv + sizeof(spirv_header)};
    const instruction_iterator end{spirv + size};
    while (it < end) {
      J_INFO("  {} {}", it->opcode(), it->word_count());
      ++it;
    }
  }
}
