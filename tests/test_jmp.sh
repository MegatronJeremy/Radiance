#!/bin/bash

assembler simple.s -o simple.o
assembler simple2.s -o simple2.o
assembler handler.s -o handler.o
linker simple.o simple2.o handler.o -o bin.o -place=text@0x40000000 -place=text2@0xFFF -hex
emulator bin.o
