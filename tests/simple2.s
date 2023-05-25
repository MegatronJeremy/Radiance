.global continue
.section text2
continue:
    st %r1, my_data
	beq %r0, %r1, end
	ret
end:
	halt
.end
