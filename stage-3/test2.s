# stage-3/test2.s

# Copyright (C) 2012 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

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
	RET

.data
	.byte	'!', '_'
	.zero	35	
	.byte	0x22
	.int	'Fish'
