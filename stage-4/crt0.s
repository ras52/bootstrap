# crt0.s

# Copyright (C) 2012, 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

####	#  Function:	void _start()
	#
	#  The ELF entry point.
_start:
	XORL	%ebp, %ebp
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	$__io_flush, %eax
	PUSH	%eax
	CALL	atexit
	POP	%eax

	LEA	8(%ebp), %eax		# argv
	PUSH	%eax
	PUSH	4(%ebp)			# argc
	CALL	main
	PUSH	%eax
	CALL	exit
	HLT

