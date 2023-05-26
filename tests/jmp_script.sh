#!/bin/bash

assembler simple.s -o simple.o
assembler simple2.s -o simple2.o
linker simple.o simple2.o -o bin.o --place=text@0x40000000 --place=text2@0xFFFF --hex
emulator bin.o
