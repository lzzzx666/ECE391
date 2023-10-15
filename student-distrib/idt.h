#ifndef IDT_H
#define IDT_H
#include "x86_desc.h"
#include "lib.h"
extern idt_desc_t idt[NUM_VEC];
extern long handlers[];
enum exception_type{DIVIDE_ERROR,DEBUG_EXCEPTION,NMI_INTERRUPT,BREAKPOINT,OVERFLOW,BOUND_RANGE_EXCEEDED,INVALID_OPCODE,DEVICE_NOT_AVAILABLE,DOUBLE_FAULT,COPROCESSOR_SEGMENT_OVERRUN,INVALID_TSS,SEGMENT_NOT_PRESENT,STACK_SEGMENT_FAULT,GENERAL_PROTECTION,PAGE_FAULT,INTEL_RESERVED,FLOATING_POINT_ERROR,ALIGNMENT_CHECK,MACHINE_CHECK,SIMD_FLOATING_POINT_EXCEPTION};
extern void idt_init();
extern void set_idt_entry(idt_desc_t idt[], int index);
void exe_exception(enum exception_type type);

#endif
