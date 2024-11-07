	# ZLoader header
.global start
.section .text
	.long 0			# .text start/len
	.long 0
	.long 0xdeadbeef	# .bss start/len
	.long 0
	.long 0			# unknown, must point into RAM
	.long upx_main
