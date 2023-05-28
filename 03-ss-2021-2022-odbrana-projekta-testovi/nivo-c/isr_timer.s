# file: isr_timer.s

.extern terminal_out

.section isr
# prekidna rutina za tajmer
.equ line_feed, 0xA
.equ carriage_return, 0xD
.equ message_len, message_end - message_start
.global isr_timer
isr_timer:
  push r0
  push r1
  ldr r1, $0
loop:
  ldr r0, [r1 + message_start]
  str r0, terminal_out 
  ldr r0, $1
  add r1, r0
  ldr r0, $message_len
  cmp r1, r0
  jne loop
  ldr r0, $line_feed
  str r0, terminal_out
  ldr r0, $carriage_return
  str r0, terminal_out
  pop r1
  pop r0
  iret
message_start:
.ascii "timer interrupt"
message_end:

.end
