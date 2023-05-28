# file: ivt.s

.extern isr_reset, isr_timer, isr_terminal

.section ivt
.word isr_reset
.skip 2 # isr_error
.word isr_timer
.word isr_terminal
.skip 8
.end
