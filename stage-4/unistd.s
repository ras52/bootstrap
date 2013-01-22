# unistd.s

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.data
####	#  Variable:	int errno;
.globl errno
errno:
	.int	0


.text

####	#  Function:	ssize_t write(int fd, const void *buf, size_t count);
	#
.globl write
write:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	16(%ebp), %edx
	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$4, %eax		# 4 == __NR_write
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L1

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L1:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	ssize_t read(int fd, void *buf, size_t count);
	#
.globl read
read:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	16(%ebp), %edx
	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$3, %eax		# 3 == __NR_read
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L2

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L2:
	POP	%ebx
	POP	%ebp
	RET
	
