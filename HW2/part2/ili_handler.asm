.globl my_ili_handler

.text
.align 4, 0x90
my_ili_handler:

	pushq	%rax
	pushq	%rbx
	pushq	%rcx
	pushq	%rdx
	pushq	%rdi
	pushq	%rsi
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	pushq	%rbp

	movq	120(%rsp), %rdi
	mov	(%rdi), %rdi

	cmp	$0x0F, %dil
	jne	go_to_function
	shr	$8, %rdi
	add	$1, 120(%rsp)
go_to_function:
	add	$1, 120(%rsp)
	call	what_to_do

	popq	%rbp
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rsi
	popq	%rdi
	popq	%rdx
	popq	%rcx
	popq	%rbx

	cmp	$0, %rax
	je	original_handler
	movq	%rax, %rdi
	popq	%rax
	iretq
original_handler:
	popq	%rax
end:
	jmp	*old_ili_handler
