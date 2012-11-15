# exit.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void _error()
	#
	#  All library error handling is done here.  
	#  (Note we can JMP here instead of CALLing it, as we never RET.)
_error:
	MOVL	$1, %ebx
	JMP	.L1


####	#  Function:	void _exit(int status)
	#
	#  Terminate program execution with given status.
_exit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %ebx
.L1:
	MOVL	$1, %eax
	INT	$0x80
	HLT
