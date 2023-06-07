ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o mathAdd.o mathAdd.s
${ASSEMBLER} -o mathDiv.o mathDiv.s
${ASSEMBLER} -o mathMul.o mathMul.s
${ASSEMBLER} -o mathSub.o mathSub.s
${ASSEMBLER} -o handler.o handler.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_software.o isr_software.s

${LINKER} -relocatable \
  -o math.o \
  mathDiv.o mathAdd.o mathMul.o mathSub.o
${LINKER} -relocatable \
  -o isr.o \
  isr_timer.o isr_terminal.o isr_software.o
${LINKER} -relocatable \
  -o prog.o \
  main.o math.o
${LINKER} -hex \
  -o program.hex \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  prog.o isr.o handler.o
${EMULATOR} program.hex
