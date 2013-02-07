# exit.s

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void _error()
	#
	#  All library error handling is done here.  
	#  (Note we can JMP here instead of CALLing it, as we never RET.)
_error:
	MOVL	$1, %eax
	PUSH	%eax
	CALL	exit
	HLT


####	#  Function:	void exit(int status)
	#
	#  Clear up streams and terminate program execution with given status.
exit:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	stdout, %eax
	PUSH	%eax
	CALL	fflush
	POP	%eax
	MOVL	stderr, %eax
	PUSH	%eax
	CALL	fflush
	POP	%eax
	PUSH	8(%ebp)
	CALL	_exit
	HLT
