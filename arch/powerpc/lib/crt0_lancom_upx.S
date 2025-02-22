.section .text.crt0

	# ZLoader header
header:
	.long _start		# .text start/len
	.long _text_size
	.long 0			# .bss start/len
	.long 0
	.long _text		# unknown, must point into RAM
	.long _start

.global _start
_start:
	lis     %r1, _stack@h           # Set the stack pointer
	b	upx_main
