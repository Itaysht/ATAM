.global	keyMixing, byteSubstitution, shiftRows, cipher

.section	.data
sbox:	.byte	99,124,119,123,-14,107,111,-59,48,1,103,43,-2,-41,-85,118
		.byte	-54,-126,-55,125,-6,89,71,-16,-83,-44,-94,-81,-100,-92,114,-64
		.byte	-73,-3,-109,38,54,63,-9,-52,52,-91,-27,-15,113,-40,49,21
		.byte	4,-57,35,-61,24,-106,5,-102,7,18,-128,-30,-21,39,-78,117
		.byte	9,-125,44,26,27,110,90,-96,82,59,-42,-77,41,-29,47,-124
		.byte	83,-47,0,-19,32,-4,-79,91,106,-53,-66,57,74,76,88,-49
		.byte	-48,-17,-86,-5,67,77,51,-123,69,-7,2,127,80,60,-97,-88
		.byte	81,-93,64,-113,-110,-99,56,-11,-68,-74,-38,33,16,-1,-13,-46
		.byte	-51,12,19,-20,95,-105,68,23,-60,-89,126,61,100,93,25,115
		.byte	96,-127,79,-36,34,42,-112,-120,70,-18,-72,20,-34,94,11,-37
		.byte	-32,50,58,10,73,6,36,92,-62,-45,-84,98,-111,-107,-28,121
		.byte	-25,-56,55,109,-115,-43,78,-87,108,86,-12,-22,101,122,-82,8
		.byte	-70,120,37,46,28,-90,-76,-58,-24,-35,116,31,75,-67,-117,-118
		.byte	112,62,-75,102,72,3,-10,14,97,53,87,-71,-122,-63,29,-98
		.byte	-31,-8,-104,17,105,-39,-114,-108,-101,30,-121,-23,-50,85,40,-33
		.byte	-116,-95,-119,13,-65,-26,66,104,65,-103,45,15,-80,84,-69,22

.section	.text
keyMixing:
	xor	%r8, %r8
	xor	%rax, %rax
loop_HW2:
	movb	(%rdi, %r8), %al
	movb	(%rsi, %r8), %dl
	xorb	%dl, %al
	movb	%al, (%rdi, %r8)
	inc	%r8
	cmp $16, %r8
	jne loop_HW2
	xor	%rax, %rax
	ret


byteSubstitution:
	xor	%r8, %r8
	leaq	sbox(%rip), %rdx
	xor	%rax, %rax
loop2_HW2:
	movb	(%rdi, %r8), %al
	movb	(%rdx ,%rax, 1), %al
	movb	%al, (%rdi, %r8)
	inc	%r8
	cmp $16, %r8
	jne loop2_HW2
	xor	%rax, %rax
	ret


shiftRows:
	xor	%r8, %r8
	mov	$4, %r8
	movl	(%rdi, %r8, 1), %eax
	ror	$8, %eax
	movl	%eax, (%rdi, %r8, 1)
	mov	$8, %r8
	movl	(%rdi, %r8, 1), %eax
	ror	$16, %eax
	movl	%eax, (%rdi, %r8, 1)
	mov	$12, %r8
	movl	(%rdi, %r8, 1), %eax
	ror	$24, %eax
	movl	%eax, (%rdi, %r8, 1)
	xor	%rax, %rax
	ret


cipher:
	push	%rbp
	mov	%rsp, %rbp
	subq	$40, %rsp
	movq	%rdi, %r11
	xor	%r10, %r10
	movq	%rax, -16(%rbp)
	movq	%rdx, -8(%rbp)
cipher_loop_HW2:
	movq	%rdi, %r12
	xor	%r9, %r9
	call	keyMixing
cipher_Nr_HW2:
	cmp	$9, %r9
	je	cipher_last_HW2
	call	byteSubstitution
	call	shiftRows
	movq	%r9, -24(%rbp)
	movq	%r10, -32(%rbp)
	movq	%rsi, -40(%rbp)
	call	mixColumns
	movq	%r12, %rdi
	movq	-24(%rbp), %r9
	movq	-32(%rbp), %r10
	movq	-40(%rbp), %rsi
	call	keyMixing
	inc	%r9
	jmp cipher_Nr_HW2
cipher_last_HW2:
	call	byteSubstitution
	call	shiftRows
	call	keyMixing
	movq	-8(%rbp), %rdx
	inc	%r10
	add	$16, %rdi
	cmp	%r10, %rdx
	jne	cipher_loop_HW2
	movq	%r11, %rdi
	movq	-16(%rbp), %rax
	leave
	ret
