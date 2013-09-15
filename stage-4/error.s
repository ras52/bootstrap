# error.s  --  bootstrap code for error handling

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


####	#  Function:	int atexit( void (*fn)(void) )
	#
	#  Dummy function that does nothing.
atexit:
	RET


####	#  Function:	void exit(int status)
	#
	#  Clear up streams and terminate program execution with given status.
exit:
	PUSH	%ebp
	MOVL	%esp, %ebp

	PUSH	8(%ebp)
	CALL	_exit
	HLT
