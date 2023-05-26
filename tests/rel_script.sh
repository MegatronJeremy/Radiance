#!/bin/bash

assembler handler.s -o handler.o
assembler -o main.o main.s
linker main.o handler.o -o programrel.o --relocatable
linker programrel.o -o program.o --place=my_code_main@0x40000000 \
  --place=my_code_handler@0x4000F000 --hex
emulator program.o
