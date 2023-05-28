# file: isr_terminal.s

.extern my_counter

.section isr
# prekidna rutina za terminal
.global isr_terminal
isr_terminal:
  push r0
  push r1
  ldr r0, 0xFF02 # term_in
  ldr r1, $2 # character_offset
  add r0, r1
  str r0, 0xFF00 # term_out
  ldr r0, %my_counter # pcrel
  ldr r1, $1
  add r0, r1
  str r0, my_counter # abs
  pop r1
  pop r0
  iret
  
.end
