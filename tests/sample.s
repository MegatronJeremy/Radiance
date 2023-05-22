# file: handler.s
.equ term_out, 0xFFFFFF00
.equ term_in, 0xFFFFFF04
.equ ascii_code, 84 # ascii(’T’)
.extern my_counter
.global handler
.section my_code_handler
.skip 8
handler:
    .word 0xABCD, 0xBC, 0xAABBCCDD
    .skip 2
    push %r1
    halt
    push %r2
    csrrd %cause, %r1
    int
.section text
    .ascii "Q PREDICTED THIS"
    ld $2, %r2
    beq %r1, %r2, my_isr_timer
    .word handler
    ld $3, %r2
    beq %r1, %r2, my_isr_terminal
# obrada prekida od tajmera
my_isr_timer:
    ld $ascii_code , %r1
    halt
    st %r1, term_out
    int
    jmp finish
# obrada prekida od terminala
.section random_section
my_isr_terminal:
    ld term_in, %r1
    st %r1, term_out
    ld my_counter, %r1
    ld $1, %r2
    add %r2, %r1
    st %r1, my_counter
finish:
    pop %r2
    pop %r1
    int
    iret
    push %r15
    push %pc
    pop %sp
    iret
    add %r15, %r14
    sub %r15, %r14
    call my_code_handler
.end
