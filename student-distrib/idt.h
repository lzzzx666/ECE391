#ifndef IDT_H
#define IDT_H
#include "x86_desc.h"
#include "lib.h"
#include "systemcall.h"
#include "types.h"

/*the macros for useful value*/
#define SYS_CALL_VEC 0x80  // 0x80 is the vector number for system call
#define INT_START 0x20     // 0x20 is the beginning number for interrupt
#define INT_END 0x2F       // 0x2F is the end number for interrupt
#define MAX_EXCEPTION 0x13 // 0x13 is the maximum exception vector that is used
#define IDT_NUMBER 256     // 256 is the number of idt entries
#define DPL_0 0            // 0 is the 0 dpl
#define DPL_3 3            // 0 is the 0 dpl
#define INT_KEYBOARD 0x21  // 0x21 is the vector number for the keyboard
#define INT_RTC 0x28       // 0x28 is the vector number for the RTC
#define INT_PIT 0x20        //the interrupt for pit
#define PRESENT 1          // 1 to indicate the handler is present
// #define INT_GATE  0xE  //0xE is the interrupt gate
// #define TRAP_GATE  0xF  //0xE is the interrupt gate

/*the whole interrupt descriptor table*/
extern idt_desc_t idt[NUM_VEC];

/*the handler function address of different exception/interrupt/system call*/
extern long handlers[];

/*the vector number of different exception/interrup/system call*/
enum idt_type
{
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
    KEYBOARD = 0x21,        // 0x21 is the vector number of keyboard
    PIT = 0x20,             // 0x20 is the vector number for pit handler
    REAL_TIME_CLOCK = 0x28, // 0x28 is the vector number of RTC
    SYSTEM_CALL = 0x80      // 0x80 is the system call entry

};

/*it will initialize the IDT*/
extern void idt_init();

/*the helper function to set every entry in the IDT*/
extern void set_idt_entry(idt_desc_t idt[], int index);

/*the main part of the handler function*/
void exc_ir_handler(enum idt_type type);

#endif
