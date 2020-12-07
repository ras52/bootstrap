# imath.s  --  functions for integer maths

# Copyright (C) 2013 Richard Smith <richard@ex-parrot.com>
# All rights reserved.

.text

####	#  Function:  int abs(int i);
	#
	#  The C library's abs() function.
.globl	abs
abs:
	PUSH	%ebp
	MOVL	%esp, %ebp
	MOVL	8(%ebp), %eax

	#  Implementation without branching
	CDQ				# sign extend into %edx:%eax
	XORL	%edx, %eax
	SUBL	%edx, %eax

	POP	%ebp
	RET


####	#  Function:	unsigned __mul_add( unsigned* val, 
	#                                   unsigned mul, unsigned add );
	# 
	#  Calculate *val = *val * mul + add and return the carry bits
	#  (i.e. the high 32 bits).  This is used in strtoul().
.globl	__mul_add
__mul_add:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	8(%ebp), %ecx
	MOVL	(%ecx), %eax
	MULL	12(%ebp)		# %edx:%eax <= *val * mul (unsigned)

	XORL	%ecx, %ecx		# do first as it clears CF
	ADDL	16(%ebp), %eax		# %eax += add;  sets CF
	ADCL	%ecx, %edx		# %edx += CF (%ecx is zero)
	
	MOVL	8(%ebp), %ecx
	MOVL	%eax, (%ecx)		# Update *val
	MOVL	%edx, %eax		# Return carry bits from %edx

	POP	%ebp
	RET


####	#  Function:	void __add64( unsigned* hi1, unsigned* low1,
	#                             unsigned  hi2, unsigned  low2 );
	# 
	#  Treat hi1:low1 and h12:low2 as 64-bit integers and add them. 
.globl	__add64
__add64:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	12(%ebp), %ecx
	MOVL	20(%ebp), %eax
	ADDL	%eax, (%ecx)		# sets CF

	MOVL	8(%ebp), %ecx
	MOVL	16(%ebp), %eax
	ADCL	%eax, (%ecx)		# adds in CF

	POP	%ebp
	RET

####	#  Function:	void __sub64( unsigned* hi1, unsigned* low1,
	#                             unsigned  hi2, unsigned  low2 );
	# 
	#  Treat hi1:low1 and h12:low2 as 64-bit integers and subtract the 
	#  second from the first.. 
.globl	__sub64
__sub64:
	PUSH	%ebp
	MOVL	%esp, %ebp

	MOVL	12(%ebp), %ecx
	MOVL	20(%ebp), %eax
	SUBL	(%ecx), %eax		# sets CF

	MOVL	8(%ebp), %ecx
	MOVL	16(%ebp), %eax
	SBBL	(%ecx), %eax		# subtracts CF too

	POP	%ebp
	RET

