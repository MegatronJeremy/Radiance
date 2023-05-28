# file: isr_terminal.s

.extern my_counter

.global terminal_out, terminal_in

.section isr
# prekidna rutina za terminal
.equ terminal_out, 0xFF00
.equ terminal_in, 0xFF02
.equ character_offset, 2
.global isr_terminal
isr_terminal:
  push r0
  push r1
  ldr r0, terminal_in
  ldr r1, $character_offset
  add r0, r1
  str r0, terminal_out
  ldr r0, %my_counter # pcrel
  ldr r1, $1
  add r0, r1
  str r0, my_counter # abs
  pop r1
  pop r0
  iret
  
.end
