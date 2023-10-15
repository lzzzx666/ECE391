#ifndef IDT_H
#define IDT_H
#include "x86_desc.h"
#include "lib.h"
#include "systemcall.h"

/*the whole interrupt descriptor table*/
extern idt_desc_t idt[NUM_VEC];

/*the handler function address of different exception/interrupt/system call*/
extern long handlers[];

/*the vector number of different exception/interrup/system call*/
enum idt_type{
    DIVIDE_ERROR,
    DEBUG_EXCEPTION,
    NMI_INTERRUPT,
    BREAKPOINT,
    OVERFLOW,
    BOUND_RANGE_EXCEEDED,
    INVALID_OPCODE,
    DEVICE_NOT_AVAILABLE,
    DOUBLE_FAULT,
    COPROCESSOR_SEGMENT_OVERRUN,
    INVALID_TSS,
    SEGMENT_NOT_PRESENT,
    STACK_SEGMENT_FAULT,
    GENERAL_PROTECTION,
    PAGE_FAULT,
    INTEL_RESERVED,
    FLOATING_POINT_ERROR,
    ALIGNMENT_CHECK,
    MACHINE_CHECK,
    SIMD_FLOATING_POINT_EXCEPTION,
    KEYBOARD=0x21,
    REAL_TIME_CLOCK=0x28,
    SYSTEM_CALL=0x80
    };

/*it will initialize the IDT*/
extern void idt_init();

/*the helper function to set every entry in the IDT*/
extern void set_idt_entry(idt_desc_t idt[], int index);

/*the main part of the handler function*/
void exe_handler(enum idt_type type);

#endif
