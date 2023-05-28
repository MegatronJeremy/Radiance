# file: isr_user2.s

.extern value0

.section isr
# prekidna rutina za slobodni ulaz 0 - OVO JE KORISTENO KAO GLAVNA PREKIDNA RUTINA
.global isr_user0
isr_user0:
  push %r2
  push %r1
  ld $0xABCD, %r2
  ld $value0, %r1
  st %r2, [%r1]
  pop %r1
  pop %r2
  iret
  
.end
