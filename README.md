# Radiance

A two-pass assembler, linker and emulator for a 32-bit ARM-like assembly language, modeled on the GNU
toolchain. Built from scratch in C++ for the System Software course at ETF Belgrade (2022/2023).

## Tools

| Tool | What it does |
| --- | --- |
| `assembler` | Two-pass assembler with a Flex/Bison front end. Handles sections, `.global`/`.extern`, `.word`, `.skip`, `.equ` expressions that are resolved once their symbols are known, and literal pools for constants that don't fit in an instruction. Emits relocatable object files in an ELF32-style format |
| `linker` | Merges object files, resolves symbols and applies relocations. `-relocatable` links into another object file, `-hex` produces a memory image, and `-place=section@address` pins sections to addresses |
| `emulator` | Runs the memory image on an emulated CPU with control and status registers, a timer and a memory-mapped terminal. Interrupts are prioritized (software and bad instruction first, then timer, then terminal) and can be masked individually |
| `readobj` | Dumps the sections, symbols and relocations of an object file |

## Example

```asm
.equ init_sp, 0xFFFFFF00
.global my_data
.section text
    ld $init_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler
    ld my_data, %r1
    halt
my_data:
    .word 5
.end
```

```sh
assembler -o main.o main.s
assembler -o handler.o handler.s
linker -hex -o program.hex -place=text@0x40000000 main.o handler.o
emulator program.hex
```

## Building

Requires `g++`, `make`, `flex` and `bison` on Linux.

```sh
./build.sh    # generates the lexer and parser, then builds and installs the four tools to /usr/bin
./clean.sh
```

`tests/` has small programs and scripts, and `01-ss-2022-2023-projekat-odbrana-testovi/` has the
course's acceptance tests at three levels (`nivo-a`, `nivo-b`, `nivo-c`).
