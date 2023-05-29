# file: handler.s
.equ term_out, 0xFFFFFF00
.equ term_in, 0xFFFFFF04
.equ ascii_code, 84 # ascii(’T’)
.equ ascii_code_two, 65 # ascii('A')
.equ ascii_code_three, 66 # ascii('B')
.extern my_counter
.global handler
.section my_code_handler
handler:
    push %r1
    push %r2
    csrrd %cause, %r1
    ld $1, %r2
    beq %r1, %r2, my_isr_bad_instr
    ld $2, %r2
    beq %r1, %r2, my_isr_timer
    ld $3, %r2
    beq %r1, %r2, my_isr_terminal
    ld $4, %r2
    beq %r1, %r2, my_isr_sw
# obrada prekida od tajmera
my_isr_timer:
    ld $ascii_code, %r1
    st %r1, term_out
    ld $1, %r8
    add %r8, %r9
    jmp finish
# obrada softverskog prekida
my_isr_sw:
    ld $ascii_code_two, %r1
    st %r1, term_out
    ld $1, %r8
    add %r8, %r5
    jmp finish
# obrada prekida lose instrukcije
my_isr_bad_instr:
    ld $ascii_code_three, %r1
    st %r1, term_out
    ld $1, %r8
    add %r8, %r6
    jmp finish
# obrada prekida od terminala
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
    iret
.skip 10

.end
