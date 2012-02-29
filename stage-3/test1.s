_start:
	MOVL	%esp, %ebp

	#  Call exit(0)
	XORL	%eax, %eax
	PUSH	%eax
	CALL	exit
	POP	%eax

	MOVL	$1, %ebx
	MOVL	$1, %eax	# __NR_exit
	INT	$0x80
