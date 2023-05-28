.global out
.global continue
.section text
continue:
    st %r1, my_data
	bgt %r0, %r1, end
	ret
out:
    jmp label
.end
