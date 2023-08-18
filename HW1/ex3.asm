.global _start

.section .text
_start:
	lea (array1), %r8
	movl (%r8), %eax
	lea (array2), %r9
	movl (%r9), %ebx
	xor %ecx, %ecx
	xor %r10, %r10
	xor %r11, %r11
	xor %r12, %r12

loop_HW1:
	cmp $0, %eax
	je finish_two_HW1
	cmp $0, %ebx
	je finish_one_HW1

	cmpl %eax, %ebx
	jle one_bigger_HW1
two_bigger_HW1:
	cmp %ebx, %ecx
	je two_bigger_2_HW1
	movl %ebx, mergedArray(,%r12, 4)
	movl %ebx, %ecx
	inc %r12

two_bigger_2_HW1:
	inc %r11
	movl array2(,%r11, 4), %ebx
	jmp loop_HW1

one_bigger_HW1:
	cmp %eax, %ecx
	je one_bigger_2_HW1
	movl %eax, mergedArray(, %r12, 4)
	movl %eax, %ecx
	inc %r12

one_bigger_2_HW1:
	inc %r10
	movl array1(, %r10, 4), %eax
	jmp loop_HW1

finish_two_HW1:
	cmp $0, %ebx
	je end_HW1
	cmp %ebx, %ecx
	je finish_two_2_HW1
	movl %ebx, mergedArray(, %r12, 4)
	movl %ebx, %ecx
	inc %r12

finish_two_2_HW1:
	inc %r11
	movl array2(, %r11, 4), %ebx
	jmp finish_two_HW1

finish_one_HW1:
	cmp $0, %eax
	je end_HW1
	cmp %eax, %ecx
	je finish_one_2_HW1
	movl %eax, mergedArray(, %r12, 4)
	movl %eax, %ecx
	inc %r12

finish_one_2_HW1:
	inc %r10
	movl array1(, %r10, 4), %eax
	jmp finish_one_HW1
end_HW1:
	movl $0, mergedArray(, %r12, 4)
