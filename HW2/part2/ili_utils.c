#include <asm/desc.h>

void my_store_idt(struct desc_ptr *idtr) {
// <STUDENT FILL> - HINT: USE INLINE ASSEMBLY
	asm("SIDT %0;": "=m" (*idtr));
// </STUDENT FILL>
}

void my_load_idt(struct desc_ptr *idtr) {
// <STUDENT FILL> - HINT: USE INLINE ASSEMBLY
	asm("LIDT %0;": "=m" (*idtr));
// <STUDENT FILL>
}

void my_set_gate_offset(gate_desc *gate, unsigned long addr) {
// <STUDENT FILL> - HINT: NO NEED FOR INLINE ASSEMBLY
	gate->offset_low = (uint16_t)(addr & 0xFFFF);
	gate->offset_middle = (uint16_t)((addr >> 16) & 0xFFFF);
	gate->offset_high = (uint32_t)(addr >> 32);
// </STUDENT FILL>
}

unsigned long my_get_gate_offset(gate_desc *gate) {
// <STUDENT FILL> - HINT: NO NEED FOR INLINE ASSEMBLY
	unsigned long ans = ((unsigned long)(gate->offset_low)) | ((unsigned long)((gate->offset_middle) << 16)) | ((unsigned long)((gate->offset_high) << 32));
	return ans;
// </STUDENT FILL>
}
