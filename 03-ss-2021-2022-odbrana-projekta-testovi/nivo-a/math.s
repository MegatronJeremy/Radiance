# file: math.s

.global mathAdd, mathSub, mathMul, mathDiv

.section math
mathAdd:
  push %r1
  ld [%sp + 8], %r7
  ld [%sp + 12], %r1
  add %r1, %r7
  pop %r1
  ret

mathSub:
  push %r1
  ld [%sp + 8], %r7 # STEK: r1, pc, arg1, arg2
  ld [%sp + 12], %r1
  sub %r1, %r7
  pop %r1
  ret

mathMul:
  push %r1
  ld [%sp + 8], %r7
  ld [%sp + 12], %r1
  mul %r1, %r7
  pop %r1
  ret

mathDiv:
  push %r1
  ld [%sp + 8], %r7
  ld [%sp + 12], %r1
  div %r1, %r7
  pop %r1
  ret

.end
