.intel_syntax noprefix

.global sum
.extern printf

.data
labela_poruke:
.asciz "Sum of 1 and 3 is %d\n"

.text

.equ term_out, 0xFFFFFF00
.extern term_in
.global term_out

.global abc
.section abc

.equ term_in, 0xFFFF-0x0FFF
.equ term_in, main+4444

main:
sum:
    push rbp
    mov rbp, rsp
    
.global main
.global main

    mov edi, 1
    mov esi, 3
    call sum

.section moja_sekcija
    mov rdi, offset labela_poruke
    mov esi, eax
    call printf

    ret

.end
