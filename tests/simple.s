# file: main.s
.equ tim_cfg, 0xFFFFFF10
.equ init_sp, 0xFFFFFF00
.global my_data
.global label
.section text
	ld $0x20, %r1
	ld $0x30, %r2
    ld $0x1, %r3
    ld $0x0, %r4
label:
    ld my_data, %r1
	sub %r3, %r1
	add %r3, %r4
	call continue
	jmp label
	halt
	.skip 10
my_data:
    .word 5
.end