#pragma once

#include "hzd/utility.hpp"

namespace j::os::elf64 {
  using addr = u64_t;
  using off = u64_t;

  using byte = u8_t;
  using half = u16_t;
  using word = u32_t;
  using xword = u64_t;

  using sword = i32_t;
  using sxword = i64_t;
  using section = u16_t;

  constexpr inline i32_t ei_nident = 16;

  enum elf_class : u8_t {
    elf_class_none,
    elf_32,
    elf_64,
  };

  enum elf_ver : u8_t {
    elf_ver_none,
    elf_ver_current,
  };

  enum elf_endian : u8_t {
    elf_endian_none,
    elf_little_endian,
    elf_big_endian,
  };

  enum elf_machine : half {
    elf_machine_none,
    elf_machine_m32,
    elf_machine_sparc,
    elf_machine_i386,
    elf_machine_m68k,
    elf_machine_m88k,
    elf_machine_iamcu,
    elf_machine_i860,
    elf_machine_mips,
    elf_machine_s370,
    elf_machine_mips_rs3_le,
    // Reserved 11-14
    elf_machine_parisc = 15,
    // Reserved 16
    elf_machine_vpp500 = 17,
    elf_machine_spark32_plus,
    elf_machine_i960,
    elf_machine_ppc,
    elf_machine_ppc64,
    elf_machine_s390,
    elf_machine_spu,
    // Reserved 24-35
    elf_machine_v800 = 36,
    elf_machine_fr20,
    elf_machine_rh32,
    elf_machine_rce,
    elf_machine_aarch32,
    elf_machine_alpha,
    elf_machine_sh,
    elf_machine_sparc_v9,
    elf_machine_tricore,
    elf_machine_arc,
    elf_machine_h8_300,
    elf_machine_h8_300h,
    elf_machine_h8s,
    elf_machine_h8_500,
    elf_machine_ia_64,
    elf_machine_mips_x,
    elf_machine_coldfire,
    elf_machine_m68hc12,
    elf_machine_mma,
    elf_machine_pcp,
    elf_machine_ncpu,
    elf_machine_ndr1,
    elf_machine_starcore,
    elf_machine_me16,
    elf_machine_st100,
    elf_machine_tinyj,
    elf_machine_amd64,
    elf_machine_pdsp,
    elf_machine_pdp10,
    elf_machine_pdp11,
    elf_machine_fx66,
    elf_machine_st9plus,
    elf_machine_st7,
    elf_machine_m68hc16,
    elf_machine_m68hc11,
    elf_machine_m68hc08,
    elf_machine_m68hc05,
    elf_machine_svx,
    elf_machine_st19,
    elf_machine_vax,
    elf_machine_cris,
    elf_machine_javelin,
    elf_machine_firepath,
    elf_machine_zsp,
    elf_machine_mmix,
    elf_machine_huany,
    elf_machine_prism,
    elf_machine_avr,
    elf_machine_fr30,
    elf_machine_d10v,
    elf_machine_d30v,
    elf_machine_v850,
    elf_machine_m32r,
    elf_machine_mn10300,
    elf_machine_mn10200,
    elf_machine_pj,
    elf_machine_openrisc,
    elf_machine_arc_compact,
    elf_machine_xtensa,
    elf_machine_videocore,
    elf_machine_tmm_gpp,
    elf_machine_ns32k,
    elf_machine_tpc,
    elf_machine_snp1k,
    elf_machine_st200,
    elf_machine_ip2k,
    elf_machine_max,
    elf_machine_cr,
    elf_machine_f2mc16,
    elf_machine_msp430,
    elf_machine_blackfin,
    elf_machine_se_c33,
    elf_machine_sep,
    elf_machine_arca,
    elf_machine_unicore,
    elf_machine_excess,
    elf_machine_dxp,
    elf_machine_nios2,
    elf_machine_crx,
    elf_machine_xgate,
    elf_machine_c166,
    elf_machine_m16c,
    elf_machine_dspic30f,
    elf_machine_ce,
    elf_machine_m32c,
    // 121-130 reserved
    elf_machine_tsk3000 = 131,
    elf_machine_rs08,
    elf_machine_sharc,
    elf_machine_ecog2,
    elf_machine_score7,
    elf_machine_dsp24,
    elf_machine_videocore3,
    elf_machine_latticemic032,
    elf_machine_se_c17,
    elf_machine_ti_c6000,
    elf_machine_ti_c2000,
    elf_machine_ti_c5500,
    elf_machine_ti_arp32,
    elf_machine_ti_pru,
    // 145-159 reserved
    elf_machine_mmdsp_plus = 160,
    elf_machine_cypress_m8c,
    elf_machine_r32c,
    elf_machine_trimedia,
    elf_machine_qdsp6,
    elf_machine_i8051,
    elf_machine_stxp7x,
    elf_machine_nds32,
    elf_machine_ecog1x,
    elf_machine_maxq30,
    elf_machine_ximo16,
    elf_machine_manik,
    elf_machine_craynv2,
    elf_machine_rx,
    elf_machine_metag,
    elf_machine_mcst_elbrus,
    elf_machine_ecog16,
    elf_machine_cr16,
    elf_machine_etpu,
    elf_machine_sle9x,
    elf_machine_l10m,
    elf_machine_k10m,
    // 182 reserved
    elf_machine_aarch64 = 183,
    // 184 reserved
    elf_machine_avr32 = 185,
    elf_machine_stm8,
    elf_machine_tile64,
    elf_machine_tilepro,
    elf_machine_microblaze,
    elf_machine_cuda,
    elf_machine_tilegx,
    elf_machine_cloudshield,
    elf_machine_corea_1st,
    elf_machine_corea_2nd,
    elf_machine_arc_compact2,
    elf_machine_open8,
    elf_machine_rl78,
    elf_machine_videocores,
    elf_machine_78kor,
    elf_machine_56800ex,
    elf_machine_ba1,
    elf_machine_ba2,
    elf_machine_xcore,
    elf_machine_pic,
    elf_machine_intel_205,
    elf_machine_intel_206,
    elf_machine_intel_207,
    elf_machine_intel_208,
    elf_machine_intel_209,
    elf_machine_km32,
    elf_machine_kmx32,
    elf_machine_kmx16,
    elf_machine_kmx8,
    elf_machine_kvarc,
    elf_machine_cdp,
    elf_machine_coge,
    elf_machine_cool,
    elf_machine_norc,
    elf_machine_csr_kalimba,
    elf_machine_z80,
    elf_machine_visium,
    elf_machine_ft32,
    elf_machine_moxie,
    elf_machine_amdgpu,
    // Reserved 225-242
    elf_machine_risc_v = 243,
  };

  static_assert(elf_machine_amd64 == 62);
  static_assert(elf_machine_ti_pru == 144);
  static_assert(elf_machine_k10m == 181);
  static_assert(elf_machine_amdgpu == 224);

  enum elf_type : half {
    elf_type_none,
    elf_type_rel,
    elf_type_exec,
    elf_type_dyn,
    elf_type_core,
    elf_type_lo_os = 0xFE00,
    elf_type_hi_os = 0xFEFF,
    elf_type_lo_proc = 0xFF00,
    elf_type_hi_proc = 0xFFFF,
  };

  enum elf_osabi : u8_t {
    elf_osabi_plain,
    elf_osabi_hpux,
    elf_osabi_netbsd,
    elf_osabi_gnu,
    elf_osabi_linux_legacy,
    elf_osabi_solaris,
    elf_osabi_aix,
    elf_osabi_irix,
    elf_osabi_frishbee,
    elf_osabi_tru64,
    elf_osabi_modesto,
    elf_osabi_openbsd,
    elf_osabi_openvms,
    elf_osabi_nsk,
    elf_osabi_aros,
    elf_osabi_fenix_os,
    elf_osabi_nuxi_cloud_abi,
    elf_osabi_openvos,

    elf_osabi_arch_specific_min = 64,
    elf_osabi_arch_specific_max = 255,
  };

  enum sec_special_index : half {
    /// Slot #0 in the index - this exists in the header table,
    /// but the section itself does not.
    sec_idx_undef,

    sec_idx_lo_proc = 0xff00,
    sec_idx_hi_proc = 0xff1f,

    sec_idx_lo_os = 0xff20,
    sec_idx_hi_os = 0xff3f,

    sec_idx_abs = 0xfff1,
    sec_idx_common = 0xfff2,
    sec_idx_xindex = 0xffff,

    sec_idx_lo_reserve = 0xff00,
    sec_idx_hi_reserve = 0xffff,
  };

  enum sec_type : word {
    /// Inactive section (section at slot #0 is always NULL)
    sec_null,
    /// Section that requires no special action from the DL.
    /// E.g. code, const, and data sections are PROGBITS, as are .debug* sections.
    sec_progbits,
    /// Symbol table for static (and dynamic) symbols.
    sec_symtab,
    /// String table
    sec_strtab,
    /// Relocation records with addend
    sec_rela,
    /// Holds the unique symbol hash table,
    sec_hash,
    /// Holds key-value pairs of miscellaneous dynamic linking information.
    sec_dynamic,
    /// Metadata intended for other programs.
    sec_note,
    /// BSS (zeroed) section, has size but occupies no space on disk.
    sec_nobits,
    /// Addendless relocation records
    sec_rel,
    /// "Reserved but has unspecified semantics"
    sec_shlib,
    /// Symbol table for dynamic symbols only.
    sec_dynsym,
    /// Array of parameterless void functions called in order after preinit functions.
    sec_init_array,
    /// Array of parameterless void functions called in order before unmapping.
    sec_fini_array,
    /// Array of parameterless void functions called after inking , before init.
    sec_preinit_array,
    /// Defines a group of sections requirin special actions during dynamic linking.
    ///
    /// Only allowed for dynamically linked libraries (type == elf_type_rel)
    sec_group,
    sec_symtab_shndx,
    sec_loos = 0x60000000,
    sec_hios = 0x6fffffff,
    sec_loproc = 0x70000000,
    sec_hiproc = 0x7fffffff,
    sec_louser = 0x80000000,
    sec_hiuser = 0xffffffff,
  };

  enum sec_flag : word {
    /// Should be linked writable
    sec_flag_write = 0x00000001,
    /// Should be mapped to memory for the application.
    sec_flag_alloc = 0x00000002,
    /// Executable code
    sec_flag_execinstr = 0x00000004,
    /// May be merged with similar sections when dynamically linking.
    /// Entries that come up as duplicates during relocation may be pruned.
    /// May have the strings flag, in which case the items are null-terminated
    /// strings instead of fixed-size records.
    sec_flag_merge = 0x00000010,
    /// Null-terminated strings, possibly of wide chars. Header entity size tells
    /// the char size in bytes.
    sec_flag_strings = 0x00000020,
    /// The `info` field refers to another section index.
    sec_flag_info_link = 0x00000040,
    /// WHen combining sections or otherwise linking, order must be kept, and if info
    /// refers to another section, that is an ordering constraint.
    sec_flag_link_order = 0x00000080,
    /// Indicates that the file should not be loaded unless the runtime linker knows
    /// exactly what to do with this file.
    sec_flag_os_nonconforming = 0x00000100,
    /// This section is a member of a section group
    sec_flag_group = 0x00000200,
    /// Thread-local storage
    sec_flag_tls = 0x00000400,
    /// Compressed section, not loaded at runtime (i.e. can't have sec_flag_alloc)
    sec_flag_compressed = 0x00000800,
    /// Mask for OS-specific extensions
    sec_mask_os = 0x0ff00000,
    /// Mask for CPU-specific extensions
    sec_mask_proc = 0xf0000000,
  };

  enum seg_type : word {
    seg_null,
    /// Memmapped segment, size divisible by page align.
    seg_load,
    seg_dynamic,
    /// Specifies interpreter for the program.
    seg_interp,
    seg_note,
    seg_shlib,
    seg_phdr,
    seg_tls,
    seg_loos = 0x60000000,
    seg_hios = 0x6fffffff,
    seg_loproc = 0x70000000,
    seg_hiproc = 0x7fffffff,
  };

  enum seg_flag : word {
    seg_x         = 0x01,
    seg_w         = 0x02,
    seg_r         = 0x04,
    seg_mask_os   = 0x0ff00000,
    seg_mask_proc = 0xf0000000,
  };

  /// Flag applied to section group entries
  enum grp_flag : word {
    /// May intersect with another group in another file, must be deduplicated.
    grp_comdat = 0x00000001,
    grp_maskos = 0x0ff00000,
    grp_maskproc = 0xf0000000,
  };
}
