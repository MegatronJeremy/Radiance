# file: main.s

.extern mathAdd, mathSub, mathMul, mathDiv

.global my_start

.global value0, value1, value2, value3, value4, value5, value6

.section my_code
my_start:
  ld $0xFEFE, %sp # init SP

  ld $isr_user0, %r7  # ivt entry for isr_user0
  csrwr %r7, %handler
  int

  ld $0, %r7
  push %r7
  ld $1, %r7
  push %r7
  call mathAdd # pc <= mathAdd
  st %r7, value1

  ld $1, %r7
  push %r7
  ld $1, %r7
  push %r7
  call mathAdd # pc <= pc + (mathAdd - pc) ~ mathAdd
  st %r7, value2

  ld $8, %r7
  push %r7
  ld $11, %r7
  push %r7
  ld $2, %r7
  ld $destinations, %r1
  add %r1, %r7
  call mathSub # pc <= mem16[r0] ~ mem16[2 + destinations] ~ mathSub
  st %r7, value3

  ld $2, %r7
  push %r7
  ld $2, %r7
  push %r7
  ld $4, %r7
  call mathMul # pc <= mem16[r0 + destinations] ~ mathMul
  st %r7, value4

  ld $5, %r7
  push %r7
  ld $25, %r7
  push %r7
  ld $6, %r7
  ld $destinations, %r1
  add %r1, %r7
  ld [%r7], %r7
  call mathDiv # pc <= r0 ~ mem16[6 + destinations] ~ mathDiv
  st %r7, value5

  ld value0, %r7 #konacna vrednost: ABCD
  ld value1, %r1 #konacna vrednost: 1
  ld value2, %r2 #konacna vrednost: 2
  ld value3, %r3 #konacna vrednost: 3
  ld value4, %r4 #konacna vrednost: 4
  ld value5, %r5 #konacna vrednost: 5
  ld value6, %sp #konacna vrednost: 0

  halt

.section my_data
value0:
.word 0
value1:
.word 0
value2:
.word 0
value3:
.word 0
value4:
.word 0
value5:
.word 0
value6:
.word 0
destinations:
.word mathAdd
.word mathSub
.word mathMul
.word mathDiv

.end
