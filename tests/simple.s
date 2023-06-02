# file: main.s
.equ tim_cfg, 0xFFFFFF10
.equ init_sp, 0xFFFFFF00
.global my_data
.global label
.global end
.section text
    ld $init_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler

	ld $0x20, %r1
	st %r1, my_data
	ld $0x30, %r2
	ld $continue2, %r9
    ld $0x1, %r3
    ld $0x0, %r4
label:
    ld my_data, %r1
	sub %r3, %r1
	add %r3, %r4
	call continue
	jmp out
	halt
	.skip 10
my_data:
    .word 5
continue2:
    st %r1, my_data
	bgt %r0, %r1, end
	ret
end:
	halt
.end
