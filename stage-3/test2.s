.data
t1:
.hex_bytes	00 01 02 03
t2:
	JMP fish

.text
exit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %ebx
	MOVL	$1, %eax		# __NR_exit
	INT	$0x80

	POP	%ebx
	POP	%ebp
	RET
