#pragma once

/* Standard ELF types.  */

#include <cstdint>
#include <iostream>

using namespace std;

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;


/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (8)

struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT] = {0x7F, 'E', 'L', 'F', 0x00, 0x00, 0x00,
                                        0x00};    /* Magic number and other info */
    Elf32_Half e_type{};            /* Object file type */
    Elf32_Addr e_entry{};        /* Entry point virtual address */
    Elf32_Off e_phoff{};        /* Program header table file offset */
    Elf32_Off e_shoff{};        /* Section header table file offset */
    Elf32_Half e_ehsize{};        /* ELF header size in bytes */
    Elf32_Half e_phentsize{};        /* Program header table entry size */
    Elf32_Half e_phnum{};        /* Program header table entry count */
    Elf32_Half e_shentsize{};        /* Section header table entry size */
    Elf32_Half e_shnum{};        /* Section header table entry count */
    Elf32_Half e_shstrndx{};        /* Section header string table index */
};

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define    ELFMAG        "\177ELF"

/* Legal values for e_type (object file type).  */

#define ET_NONE        0        /* No file type */
#define ET_REL        1        /* Relocatable file */
#define ET_EXEC        2        /* Executable file */


/* Symbol table entry.  */

struct Elf32_Sym {
    Elf32_Word st_name{};        /* Symbol name (string tbl index) */
    Elf32_Addr st_value{};        /* Symbol value */
    unsigned char st_info{};        /* Symbol type and binding */
    Elf32_Section st_shndx{};        /* Section index */
};

/* Legal values for ST_BIND subfield of st_info (symbol binding).  */

#define STB_LOCAL    0        /* Local symbol */
#define STB_GLOBAL    1        /* Global symbol */

/* Legal values for ST_TYPE subfield of st_info (symbol type).  */

#define STT_NOTYPE    0        /* Symbol type is unspecified */
#define STT_SECTION    3        /* Symbol associated with a section */

/* How to extract and insert information held in the st_info field.  */

#define ELF32_ST_BIND(val)        (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)        ((val) & 0xf)
#define ELF32_ST_INFO(bind, type)    (((bind) << 4) + ((type) & 0xf))


/* Relocation table entry with addend (in section of type SHT_RELA).  */

struct Elf32_Rela {
    Elf32_Addr r_offset{};        /* Address */
    Elf32_Word r_info{};            /* Relocation type and symbol index */
    Elf32_Sword r_addend{};        /* Addend */
};

/* Relocation types.  */

#define R_NONE        0    /* No reloc */
#define R_PC32        2    /* PC relative 32 bit signed */
#define R_32        10    /* Direct 32 bit zero extended */
#define R_32S        11    /* Direct 32 bit sign extended */

/* How to extract and insert information held in the r_info field.  */

#define ELF32_R_SYM(val)        ((val) >> 8)
#define ELF32_R_TYPE(val)        ((val) & 0xff)
#define ELF32_R_INFO(sym, type)        (((sym) << 8) + ((type) & 0xff))


/* Section header.  */

struct Elf32_Shdr {
    Elf32_Word sh_name{};        /* Section name (string tbl index) */
    Elf32_Word sh_type{};        /* Section type */
    Elf32_Word sh_flags{};        /* Section flags */
    Elf32_Addr sh_addr{};        /* Section virtual addr at execution */
    Elf32_Off sh_offset{};        /* Section file offset */
    Elf32_Word sh_size{};        /* Section size in bytes */
    Elf32_Word sh_link{};        /* Link to another section */
    Elf32_Word sh_info{};        /* Additional section information */
    Elf32_Word sh_addralign{};        /* Section alignment */
    Elf32_Word sh_entsize{};        /* Entry size if section holds table */
};

/* Special section indices.  */

#define SHN_UNDEF    0        /* Undefined section */
#define SHN_ABS        0xfff1        /* Associated symbol is absolute */

/* Legal values for sh_type (section type).  */

#define SHT_NULL      0        /* Section header table entry unused */
#define SHT_PROGBITS      1        /* Program data */
#define SHT_SYMTAB      2        /* Symbol table */
#define SHT_STRTAB      3        /* String table */
#define SHT_RELA      4        /* Relocation entries with addends */
#define SHT_NOBITS      8        /* Program space with no data (bss) */

/* Legal values for sh_flags (section flags).  */

#define SHF_WRITE         (1 << 0)    /* Writable */
#define SHF_ALLOC         (1 << 1)    /* Occupies memory during execution */
#define SHF_EXECINSTR         (1 << 2)    /* Executable */
#define SHF_INFO_LINK         (1 << 6)    /* `sh_info' contains SHT index */


struct ELF_File {
    Elf32_Ehdr elfHeader;
    uint8_t text[8]{};
    uint8_t data[88888888]{};
    uint8_t bss[8]{};
    Elf32_Sym symtab[8];
    uint8_t strtab[8]{};
    Elf32_Rela relaText[1];
    uint8_t shstrtab[8]{};
    Elf32_Shdr sectionHeaderTable[8];
};