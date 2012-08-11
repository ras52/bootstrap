.text
_start:
	MOVL	%esp, %ebp

	#  Call exit(0)
	XORL	%eax, %eax
	PUSH	%eax
	CALL	exit
	POP	%eax

# It should be safe to embed this literal in the middle of this function
.data
.hex_bytes  48 65 6C 6C 6F 2C 20 77 6F 72 6C 64 21 00

.text
	MOVL	$1, %ebx
	MOVL	$1, %eax	# __NR_exit
	INT	$0x80
	NOP NOP
.data
