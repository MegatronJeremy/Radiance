    .section rodata
msg:
    .ascii "Hi\n"
    .section text
    .global main
main:
    push %r10
    st %sp, %r10
    call getchar
    ld $10, %r1
    bne %r1, %r2, skip
    ld $msg, %r3
    call printf
skip:
    ld $0, %r1
    st %r10, %sp
    pop %r10
    ret
.end

merge:
    push %r1
