.text
exit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %ebx
	MOVL	$1, %eax		# __NR_exit
	INT	$0x80

	POP	%ebx
	LEAVE

.data
	.byte	0x21
