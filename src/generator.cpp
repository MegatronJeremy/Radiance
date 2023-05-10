#include <elf.h>
#include <stdio.h>
#include <string.h>

#define STRTAB        \
    "\0message_len"   \
    "\0message_end"   \
    "\0message_start" \
    "\0custom_entry"  \
    "\0"

#define SHSTRTAB   \
    "\0.symtab"    \
    "\0.strtab"    \
    "\0.shstrtab"  \
    "\0.rela.text" \
    "\0.data"      \
    "\0.bss"       \
    "\0"

int getOffset(char *needle, char *haystack, int haystackLen) {
    int needleLen = strlen(needle);
    char *search = haystack;
    int searchLen = haystackLen - needleLen + 1;
    for (; searchLen-- > 0; search++) {
        if (!memcmp(search, needle, needleLen)) {
            return search - haystack;
        }
    }
    return 0;
}

enum {
    SECTION_SIZE_TEXT = 46,
    SECTION_SIZE_DATA = 14,
    SECTION_SIZE_BSS = 0,
    SECTION_SIZE_STRTAB = 52,
    SECTION_SIZE_SHSTRTAB = 49,
};

enum {
    SYMTAB_NDX_UNDEF = 0,
    SYMTAB_NDX_TEXT,
    SYMTAB_NDX_DATA,
    SYMTAB_NDX_BSS,
    SYMTAB_NDX_MESSAGE_LEN,
    SYMTAB_NDX_MESSAGE_END,
    SYMTAB_NDX_MESSAGE_START,
    SYMTAB_NDX_CUSTOM_ENTRY,
};

enum {
    SECTION_NDX_UNDEF = 0,
    SECTION_NDX_TEXT,
    SECTION_NDX_RELA_TEXT,
    SECTION_NDX_DATA,
    SECTION_NDX_BSS,
    SECTION_NDX_SYMTAB,
    SECTION_NDX_STRTAB,
    SECTION_NDX_SHSTRTAB,
};

// man 5 elf
// https://man7.org/linux/man-pages/man5/elf.5.html
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
int main() {
    struct ElfFile {
        Elf32_Ehdr elfHeader;                    //       [  0 -  63] :  1 * sizeof(Elf64_Ehdr) = 1 * 64
        uint8_t text[SECTION_SIZE_TEXT];         //       [ 64 - 109] : 46 * sizeof(uint8_t) = 46 * 1
        uint8_t data[SECTION_SIZE_DATA];         //       [110 - 123] : 14 * sizeof(uint8_t) = 14 * 1
        uint8_t bss[SECTION_SIZE_BSS];           //
        Elf32_Sym symtab[8];                     // +padd [128 - 319] :  8 * sizeof(Elf64_Sym) = 8 * 24
        uint8_t strtab[SECTION_SIZE_STRTAB];     //       [320 - 371] : 52 * sizeof(uint8_t) = 52 * 1
        Elf32_Rela relaText[1];                  // +padd [376 - 399] :  1 * sizeof(Elf64_Rela) = 1 * 24
        uint8_t shstrtab[SECTION_SIZE_SHSTRTAB]; //       [400 - 448] : 49 * sizeof(uint8_t) = 49 * 1
        Elf32_Shdr sectionHeaderTable[8];        //       [456 - 967] :  8 * sizeof(Elf64_Shdr) = 8 * 64
    } elfFile = {
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html
            .elfHeader = {
                    .e_ident = {/* Magic number and other info */
                            /* [0] EI_MAG        */ 0x7F, 'E', 'L', 'F',
                            /* [4] EI_CLASS      */ ELFCLASS64,
                            /* [5] EI_DATA       */ ELFDATA2LSB,
                            /* [6] EI_VERSION    */ EV_CURRENT,
                            /* [7] EI_OSABI      */ ELFOSABI_SYSV,
                            /* [8] EI_ABIVERSION */ 0,
                            /* [9-15] EI_PAD     */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                    .e_type = ET_REL,                                                  /* Object file type */
                    .e_machine = EM_X86_64,                                            /* Required architecture */
                    .e_version = EV_CURRENT,                                           /* Object file version */
                    .e_entry = 0,                                                      /* Entry point virtual address */
                    .e_phoff = 0,                                                      /* Program header table file offset */
                    .e_shoff = (void *) &elfFile.sectionHeaderTable -
                               (void *) &elfFile, /* Section header table file offset */
                    .e_flags = 0,                                                      /* Processor-specific flags */
                    .e_ehsize = sizeof(Elf64_Ehdr),                                    /* ELF header size in bytes */
                    .e_phentsize = 0,                                                  /* Program header table entry size */
                    .e_phnum = 0,                                                      /* Program header table entry count */
                    .e_shentsize = sizeof(Elf64_Shdr),                                 /* Section header table entry size */
                    .e_shnum = 8,                                                      /* Section header table entry count */
                    .e_shstrndx = SECTION_NDX_SHSTRTAB                                 /* Section header string table index */
            },
            .text = {
                    // clang-format off
                    [0x00] = 0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, // mov rax, 1
                    [0x07] = 0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00, // mov rdi, 1
                    [0x0e] = 0x48, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00, // mov rsi, offset message_start
                    [0x15] = 0x48, 0xc7, 0xc2, 0x13, 0x00, 0x00, 0x00, // mov rdx, offset message_len
                    [0x1c] = 0x0f, 0x05,                               // syscall
                    [0x1e] = 0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60
                    [0x25] = 0x48, 0xc7, 0xc7, 0x0d, 0x00, 0x00, 0x00, // mov rdi, 13
                    [0x2c] = 0x0f, 0x05                                // syscall
                    // clang-format on
            },
            .data = "Hello World!\n",
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.symtab.html
            .symtab = {
                    [SYMTAB_NDX_UNDEF] = {
                            .st_name = 0,
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SHN_UNDEF, // *UND*
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_TEXT] = {
                            .st_name = 0,
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_TEXT,
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_DATA] = {
                            .st_name = 0,
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_DATA,
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_BSS] = {
                            .st_name = 0,
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_BSS,
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_MESSAGE_LEN] = {
                            .st_name = getOffset("message_len", STRTAB, SECTION_SIZE_STRTAB),
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SHN_ABS, // *ABS*
                            .st_value = 0x000000000000000el,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_MESSAGE_END] = {
                            .st_name = getOffset("message_end", STRTAB, SECTION_SIZE_STRTAB),
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_DATA,
                            .st_value = 0x000000000000000el,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_MESSAGE_START] = {
                            .st_name = getOffset("message_start", STRTAB, SECTION_SIZE_STRTAB),
                            .st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_DATA,
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
                    [SYMTAB_NDX_CUSTOM_ENTRY] = {
                            .st_name = getOffset("custom_entry", STRTAB, SECTION_SIZE_STRTAB),
                            .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE),
                            .st_other = STV_DEFAULT,
                            .st_shndx = SECTION_NDX_TEXT,
                            .st_value = 0x0000000000000000l,
                            .st_size = 0,
                    },
            },
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.strtab.html
            .strtab = STRTAB,
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.reloc.html
            .relaText = {
                    [0] = {.r_offset = 0x11, .r_info = ELF64_R_INFO(SYMTAB_NDX_DATA, R_X86_64_32S), .r_addend = 0x00},
            },
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.strtab.html
            .shstrtab = SHSTRTAB,
            // https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.sheader.html
            .sectionHeaderTable = {
                    [SECTION_NDX_UNDEF] = {
                            .sh_name = 0,
                            .sh_type = 0,
                            .sh_flags = 0,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = 0,
                            .sh_size = 0,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 0,
                            .sh_entsize = 0,
                    },
                    [SECTION_NDX_TEXT] = {
                            .sh_name = getOffset(".text", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_PROGBITS,
                            .sh_flags = SHF_EXECINSTR | SHF_ALLOC,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.text - (void *) &elfFile,
                            .sh_size = 0x2e,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 1,
                            .sh_entsize = 0,
                    },
                    [SECTION_NDX_RELA_TEXT] = {
                            .sh_name = getOffset(".rela.text", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_RELA,
                            .sh_flags = SHF_INFO_LINK,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.relaText - (void *) &elfFile,
                            .sh_size = 0x18,
                            .sh_link = SECTION_NDX_SYMTAB, // section header index for .symtab
                            .sh_info = SECTION_NDX_TEXT,   // section header index for .text
                            .sh_addralign = 8,
                            .sh_entsize = sizeof(Elf64_Rela),
                    },
                    [SECTION_NDX_DATA] = {
                            .sh_name = getOffset(".data", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_PROGBITS,
                            .sh_flags = SHF_WRITE | SHF_ALLOC,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.data - (void *) &elfFile,
                            .sh_size = 0x0e,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 1,
                            .sh_entsize = 0,
                    },
                    [SECTION_NDX_BSS] = {
                            .sh_name = getOffset(".bss", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_NOBITS,
                            .sh_flags = SHF_WRITE | SHF_ALLOC,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.bss - (void *) &elfFile,
                            .sh_size = 0x00,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 1,
                            .sh_entsize = 0,
                    },
                    [SECTION_NDX_SYMTAB] = {
                            .sh_name = getOffset(".symtab", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_SYMTAB,
                            .sh_flags = 0,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.symtab - (void *) &elfFile,
                            .sh_size = 0xc0,
                            .sh_link = SECTION_NDX_STRTAB,      // section header index for .strtab
                            .sh_info = SYMTAB_NDX_CUSTOM_ENTRY, // one greater than the symbol table index of the last local symbol
                            .sh_addralign = 8,
                            .sh_entsize = sizeof(Elf64_Sym),
                    },
                    [SECTION_NDX_STRTAB] = {
                            .sh_name = getOffset(".strtab", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_STRTAB,
                            .sh_flags = 0,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.strtab - (void *) &elfFile,
                            .sh_size = 0x34,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 1,
                            .sh_entsize = 0,
                    },
                    [SECTION_NDX_SHSTRTAB] = {
                            .sh_name = getOffset(".shstrtab", SHSTRTAB, SECTION_SIZE_SHSTRTAB),
                            .sh_type = SHT_STRTAB,
                            .sh_flags = 0,
                            .sh_addr = 0x0000000000000000l,
                            .sh_offset = (void *) &elfFile.shstrtab - (void *) &elfFile,
                            .sh_size = 0x31,
                            .sh_link = 0,
                            .sh_info = 0,
                            .sh_addralign = 1,
                            .sh_entsize = 0,
                    },
            }};

    FILE *fileHandle = fopen("generated.o", "wb");
    if (fileHandle != NULL) {
        fwrite(&elfFile, sizeof(struct ElfFile), 1, fileHandle);
    }

    return 0;
}

/*
    |----------------------|
    |                      |
    |      ELF Header      |
    |                      |
    |                      |
    |----------------------|
    |                      |<----+
    |        .text         |     |
    |                      |     |
    |----------------------|     |
    |        .data         |<----------+
    |----------------------|     |     |
    |                      |<-------------+
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |       .symtab        |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |                      |     |     |  |
    |----------------------|     |     |  |
    |                      |<----------------+
    |       .strtab        |     |     |  |  |
    |                      |     |     |  |  |
    |----------------------|     |     |  |  |
    |      .rela.text      |<-------+  |  |  |
    |                      |     |  |  |  |  |
    |----------------------|     |  |  |  |  |
    |                      |<-------------------+
    |      .shstrtab       |     |  |  |  |  |  |
    |                      |     |  |  |  |  |  |
    |----------------------|     |  |  |  |  |  |
    |                      |     |  |  |  |  |  |
    |                      |     |  |  |  |  |  |
    |                      | [0] |  |  |  |  |  |
    |                      |--   |  |  |  |  |  |
    |                      |     |  |  |  |  |  |
    |                      |     |  |  |  |  |  |
    |                      | [1] |  |  |  |  |  |
    |                      |-----+  |  |  |  |  |
    |                      |        |  |  |  |  |
    |                      |        |  |  |  |  |
    |                      | [2]    |  |  |  |  |
    |                      |--------+  |  |  |  |
    |                      |           |  |  |  |
    |                      |           |  |  |  |
    |                      | [3]       |  |  |  |
    | Section Header Table |-----------+  |  |  |
    |                      |              |  |  |
    |                      |              |  |  |
    |                      | [4]          |  |  |
    |                      |--------------+  |  |
    |                      |              |  |  |
    |                      |              |  |  |
    |                      | [5]          |  |  |
    |                      |--------------+  |  |
    |                      |                 |  |
    |                      |                 |  |
    |                      | [6]             |  |
    |                      |-----------------+  |
    |                      |                    |
    |                      |                    |
    |                      | [7]                |
    |                      |--------------------+
    |----------------------|
 */
