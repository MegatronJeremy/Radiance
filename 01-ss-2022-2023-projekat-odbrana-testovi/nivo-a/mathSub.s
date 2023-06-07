# file: math.s

.global mathSub

.section math
mathSub:
    push %r2
    ld [%sp + 0x08], %r1
    ld [%sp + 0x0C], %r2
    ld $handler, %r11
    sub %r2, %r1 # r1 used for the result
    pop %r2
    ret

