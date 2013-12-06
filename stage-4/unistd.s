# unistd.s  --  Linux syscalls

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
	

####	#  Function:	void _exit(int status)
	#
	#  Terminate program execution with given status.
.globl _exit
_exit:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %ebx	
	MOVL	$1, %eax		# 1 == __NR_exit
	INT	$0x80
	HLT


####	#  Function:	int open(char const* filename, int flags, int mode);
	#
.globl open
open:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	16(%ebp), %edx
	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$5, %eax		# 5 == __NR_open
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L3

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L3:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	int close(int fd);
	#
.globl close
close:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %ebx
	MOVL	$6, %eax		# 6 == __NR_close
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L4

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L4:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	int dup2(int oldfd, int newfd);
	#
.globl dup2
dup2:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$63, %eax		# 63 == __NR_dup2
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L5

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L5:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void* mmap(void *addr, size_t length, int prot, 
	#                          int flags, int fd, off_t offset);
	#
.globl mmap
mmap:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	LEA	8(%ebp), %ebx
	MOVL	$90, %eax		# 90 == __NR_mmap
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L6

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax

.L6:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	void (*signal(int signum, void (*handler)(int)))(int);
	#
.globl signal
signal:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$48, %eax		# 48 == __NR_signal
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L7

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L7:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	int kill( pid_t pid, int sig );
	#
.globl kill
kill:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$37, %eax		# 37 == __NR_kill
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L8

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L8:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	pid_t getpid();
	#
.globl getpid
getpid:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	$20, %eax		# 20 == __NR_getpid
	INT	$0x80
	#  NB the getpid syscall cannot fail.
	POP	%ebp
	RET


####	#  Function:	int execve(char* filename, char* argv[], char* envp[]);
	#
.globl execve
execve:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	16(%ebp), %edx
	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$11, %eax		# 11 == __NR_execve
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L9

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L9:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	int fork();
	#
.globl fork
fork:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	$2, %eax		# 2 == __NR_fork
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L10

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L10:
	POP	%ebp
	RET


####	#  Function:	int waitpid(int pid, int* status, int options);
	#
.globl waitpd
waitpid:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	16(%ebp), %edx
	MOVL	12(%ebp), %ecx
	MOVL	8(%ebp), %ebx
	MOVL	$7, %eax		# 7 == __NR_waitpid
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L11

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L11:
	POP	%ebx
	POP	%ebp
	RET


####	#  Function:	int unlink(char* filename);
	#
.globl unlink
unlink:
	PUSH	%ebp
	MOVL	%esp, %ebp
	PUSH	%ebx

	MOVL	8(%ebp), %ebx
	MOVL	$10, %eax		# 10 == __NR_unlink
	INT	$0x80
	CMPL	$-4096, %eax		# -4095 <= %eax < 0 for errno
	JNA	.L12

	NEGL	%eax
	MOVL	%eax, errno
	XORL	%eax, %eax
	DECL	%eax
.L12:
	POP	%ebx
	POP	%ebp
	RET



