.global _start

.section .text
_start:
	leaq (destination), %r8
	movl (num), %eax
	cmpl $0, %eax
	jge positive_HW1
	mov %eax, destination
	jmp end_HW1
positive_HW1:
	leaq (source), %rcx
	cmpq %rcx, %r8
	jl positive2_HW1
	add %rax, %rcx
	cmpq %rcx, %r8
	jle backwards_HW1
positive2_HW1:
	xor %edx, %edx
	xor %bl, %bl
	movb (source), %bl
	cmpl %eax, %edx
	jge end_HW1
loop_HW1:
	movb %bl, destination(,%edx, 1)
	inc %edx
	movb source(,%edx, 1), %bl
	cmpl %eax, %edx
	jl loop_HW1
	jmp end_HW1
backwards_HW1:
	xor %bl, %bl
	xor %edx, %edx
	dec %eax
	movb source(, %eax, 1), %bl
loop_back_HW1:
	movb %bl, destination(,%eax, 1)
	dec %eax
	movb source(, %eax, 1), %bl
	cmpl %edx, %eax
	jge loop_back_HW1
end_HW1:
