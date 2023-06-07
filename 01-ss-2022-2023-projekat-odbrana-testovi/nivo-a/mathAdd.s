# file: math.s

.global mathAdd

.section math
mathAdd:
    push %r2
    ld $10, %r3
    ld $1, %r4
    ld [%sp + 0x08], %r1
    ld [%sp + 0x0C], %r2
    add %r2, %r1 # r1 used for the result
    sub %r4, %r3
    ld value, %r10
    pop %r2
    ret
value:
    .word 0xABCDABCD
.end

