.global getchar
.section text
my_isr_timer:
    add %r15, %r14
    call getchar
