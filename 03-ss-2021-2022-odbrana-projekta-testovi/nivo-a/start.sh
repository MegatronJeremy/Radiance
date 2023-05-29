ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o math.o math.s
${ASSEMBLER} -o ivt.o ivt.s
${ASSEMBLER} -o isr_reset.o isr_reset.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_user0.o isr_user0.s
${LINKER} --hex -o program.hex ivt.o math.o main.o -place=my_code@0x40000000 -place=math@0xC0000000 -place=my_data@0xFFFF \
      isr_reset.o isr_terminal.o isr_timer.o isr_user0.o
${EMULATOR} program.hex
