# file: math.s

.global mathDiv

.section math
mathDiv:
    push %r2
    ld [%sp + 0x08], %r1
    ld [%sp + 0x0C], %r2
    ld $handler, %r9
    div %r2, %r1 # r1 used for the result
    pop %r2
    ret

