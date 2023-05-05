# file: main.s
.equ tim_cfg, 0xFFFFFF10
.equ init_sp, 0xFFFFFF00
.extern handler
.section my_code_main
	ld $init_sp, %sp
	ld $handler, %r1
	csrwr %r1, %handler
	ld $0x1, %r1
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
.end