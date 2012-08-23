.data 
fmt:	
	.string "%c: '%s'\n"


.text 
main:
	PUSH	%ebp
	MOVL	%esp, %ebp

.L1:
	CALL	next
	CMPL	$-1, %eax
	JE	.L2
	
	MOVL	$value, %eax
	PUSH	%eax
	MOVL	token, %eax
	PUSH	%eax
	MOVL	$fmt, %eax
	PUSH	%eax
	CALL	printf
	POP	%eax
	POP	%eax
	POP	%eax
	JMP	.L1

.L2:
	XORL	%eax, %eax
	LEAVE
	RET
