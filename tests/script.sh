#!/bin/bash

assembler main.s -o main.o
assembler sample.s -o sample.o
linker main.o sample.o -o binrel.o --place=text@0x40000000 --place=my_code_handler@0x4000F000 --place=random_section@0x4000 --relocatable
readobj binrel.o
