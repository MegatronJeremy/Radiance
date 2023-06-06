# file: main.s
.equ tim_cfg, 0xFFFFFF10
.equ init_sp, 0xFFFFFF00
.equ eight, four + 0x4
.equ four, 0x4
.equ a_lot, 0xFFFFF
.global main
.extern handler
.section my_code_main
    ld $init_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler

main:
    ld $0x1, %r1
    st %r1, tim_cfg

    ld $7, %r2
    ld $1, %r3


loop:
    # testing software interrupts
    int

    # testing bad instr interrupts
    .word 0xFFFFFFFF

    sub %r3, %r2
    bgt %r2, %r0, loop

    # testing addressing modes
    ld $my_counter, %r12
    ld $my_counter, %r4
    ld [%r12 + four], %r12

    ld $my_counter, %r13
    ld [%r13 + eight], %r13
    st %r12, [%r4 + eight]
    ld [%r4 + eight], %r13
    ld $eight, %r10
    ld $a_lot, %r11

    ld $my_counter, %r12
    ld [%r12 + 12], %r12

    ld $0x0, %r9

wait:
    ld my_counter, %r1
    ld $5, %r2
    bne %r1, %r2, wait
    halt
.ascii "Q PREDICTED THIS/n"
.skip 1000
.global my_counter
.section my_data
my_counter:
    .word 0
value_one:
    .word 0xa
value_two: .word 0xb
value_three: .word 0xfffffffa
.ascii "Q PREDICTED THIS/t"
.end
