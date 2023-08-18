.global _start

.section .text
_start:
	movq (root), %rdx
	testq %rdx, %rdx
	je end_HW1
	xor %r8, %r8
	movq $16, %r10
	movq $8, %r11
	movq (new_node), %r9
loop_HW1:
	movq (%rdx), %rcx
	cmpq %rcx, %r9
	cmovg %r10, %r8
	cmovl %r11, %r8
	je end_HW1
	cmpq $0, (%rdx, %r8)
	je put_HW1
	movq (%rdx, %r8), %rdx
	testq %rdx, %rdx
	jne loop_HW1
put_HW1:
	movq $new_node, (%rdx, %r8)
end_HW1:
