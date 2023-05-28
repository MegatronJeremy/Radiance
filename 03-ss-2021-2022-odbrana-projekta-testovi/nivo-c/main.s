# file: main.s

.global my_start, my_counter

.section code
.equ initial_sp, 0xFEFE
.equ timer_config, 0xFF10
my_start:
  ldr r6, $initial_sp # initial value for SP

  ldr r0, $0x1
  str r0, timer_config
wait:
  ldr r0, my_counter
  ldr r1, $20
  cmp r0, r1
  jne wait
  halt

.section my_data
my_counter:
.word 0

.end
