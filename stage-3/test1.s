.data
bar:
	.int	0x2A

foo:
	.int	0xFFFF	

.text
_start:
	MOVL	%esp, %ebp

	#  Call exit(foo)
	MOVL	foo, %eax
	CMPL	$0, %eax
	SETE	%al
	MOVZBL	%al, %eax
	MOVL	%eax, foo

	MOVL	foo, %eax
	PUSH	%eax

# It should be safe to embed this literal in the middle of this function
.data
	.string	"\"Hello,\tworld\!\""
	.byte	0xFF

.text
	CALL	exit
	HLT

.data
