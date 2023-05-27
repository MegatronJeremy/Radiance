# file: main.s
.equ tim_cfg, 0xFFFFFF10
.equ init_sp, 0xFFFFFF00
.extern handler
.section my_code_main
    ld $init_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler

    # testing addressing modes
    ld $0x4, %r12
    ld [%r12 + my_counter], %r12
    ld $my_counter, %r13
    ld [%r13 + 8], %r13

    ld $0x1, %r1
    ld $0x0, %r9
    st %r1, tim_cfg
wait:
    ld my_counter, %r1
    ld $5, %r2
    bne %r1, %r2, wait
    halt
.global my_counter
.section my_data
my_counter:
    .word 0
value_one:
    .word 0xa
value_two:
    .word 0xb
.end
