.data
bar:
	.int	0x2A

foo:
	.int	1

.text
_start:
	MOVL	%esp, %ebp

	#  Call exit(foo)
	MOVL	foo, %eax
	DECL	%eax
	MOVL	%eax, foo

	MOVL	foo, %eax
	PUSH	%eax

# It should be safe to embed this literal in the middle of this function
.data
	.hex	48 65 6C 6C 6F 2C 20 77 6F 72 6C 64 21 00

.text
	CALL	exit
	HLT

.data
