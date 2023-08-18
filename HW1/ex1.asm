.global _start

.section .text
_start:
	lea (num), %rax
	mov (%rax), %rbx
	xor %cl, %cl
loop_HW1:
	cmpq $0, %rbx
	je end_HW1
	mov %rbx, %rdx
	shr $1, %rbx
	shl $1, %rbx
	mov %rbx, %r8
	shr $1, %rbx
	cmpq %rdx, %r8
	je loop_HW1
	inc %cl
	jmp loop_HW1
end_HW1:
	mov %cl, Bool

