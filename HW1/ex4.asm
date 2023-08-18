.global _start

.section .text
_start:
	movq (head), %r8
	movq (Source), %r9
	movl (Value), %eax
	movq $4, %r10
	xor %rbx, %rbx
	xor %rcx, %rcx
	xor %r13, %r13
	jmp find_previous_of_source_HW1
after_find_of_previous_HW1:
	movq (head), %r8
	xor %r11, %r11
	cmpl (%r8), %eax
	je if_first_node_is_value_HW1
loop_HW1:
	movq %r8, %r11
	add %r10, %r8
	cmpq $0, (%r8)
	je end_HW1
	movq (%r8), %r8
	cmpl (%r8), %eax
	jne loop_HW1
	jmp loop2_HW1
if_first_node_is_value_HW1:
	mov %r9, (head)
	jmp loop2_2_HW1
loop2_HW1:
	movq %r9, (%r11, %r10)
loop2_2_HW1:
	movq (%r9, %r10), %r12
	movq (%r8, %r10), %rbx
	cmpq %rbx, %r9
	je neighbors_HW1
	movq %rbx, (%r9, %r10)
loop2_3_HW1:
	movq %r12, (%r8, %r10)
	cmpq $0, %rcx
	je if_source_is_first_HW1
	cmpq %r8, %rcx
	je end_HW1
	movq %r8, (%rcx, %r10)
	jmp end_HW1

neighbors_HW1:
	movq %r8, (%r9, %r10)
	jmp loop2_3_HW1

find_previous_of_source_HW1:
	cmpq %r8, %r9
	je after_find_of_previous_HW1
	movq %r8, %rcx
	add %r10, %r8
	movq (%r8), %r8
	jmp find_previous_of_source_HW1
if_source_is_first_HW1:
	movq %r8, (head)
end_HW1:
