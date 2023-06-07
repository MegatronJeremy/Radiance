# file: math.s

.global mathMul

.section math
mathMul:
    push %r2
    ld [%sp + 0x08], %r1
    ld [%sp + 0x0C], %r2
    mul %r2, %r1 # r1 used for the result
    pop %r2
    ret

.end
