#include "idt.h"
#include "x86_desc.h"

void idt_init()
{
    
    int i = 0; // the iterator
    for (i = 0; i < 256; i++)
    {
        idt[i].val[0]=0;
    idt[i].val[1]=0;
    if(i<=0x13){
        set_idt_entry(idt, i);
    }
    }
    lidt(idt_desc_ptr);
}
extern long divide_error;
void set_idt_entry(idt_desc_t idt[], int index)
{
    idt[index].dpl = 0;
    idt[index].present = 1;
    idt[index].seg_selector = KERNEL_CS;
    idt[index].size = 1;
    idt[index].reserved1=1;
    idt[index].reserved2=1;
    idt[index].reserved3=1;
    SET_IDT_ENTRY(idt[index], handlers[index]);
}

void exe_exception(enum exception_type type){
    
    cli();
    switch(type){

    case DIVIDE_ERROR:
        printf("divided_error!");
        break;
    case DEBUG_EXCEPTION:
        printf("debug_exception!");
        break;
    case NMI_INTERRUPT:
       printf("nmi_interrupt!");
        break;
    case BREAKPOINT:
        printf("breakpoint!");
        break;
    case OVERFLOW:
        printf("overflow!");
        break;
    case BOUND_RANGE_EXCEEDED:
        printf("bound_range_exceeded!");
        break;
    case INVALID_OPCODE:
        printf("invalid_opcode!");
        break;
    case DEVICE_NOT_AVAILABLE:
        printf("device_not_available!");
        break;
    case DOUBLE_FAULT:
        printf("double_fault!");
        break;
    case COPROCESSOR_SEGMENT_OVERRUN:
        printf("coprocessor_segment_overrun!");
        break;
    case INVALID_TSS:
        printf("invalid_tss!");
        break;
    case SEGMENT_NOT_PRESENT:
        printf("segment_not_present!");
        break;
    case STACK_SEGMENT_FAULT:
       printf("stack_segment_fault!");
        break;
    case GENERAL_PROTECTION:
        printf("general_protection!");
        break;
    case PAGE_FAULT:
        printf("page_fault!");
        break;
    case INTEL_RESERVED:
        printf("intel_reserved!");
        break;
    case FLOATING_POINT_ERROR:
        printf("floating_point_error!");
        break;
    case ALIGNMENT_CHECK:
        printf("alignment_check!");
        break;
    case MACHINE_CHECK:
        printf("machine_check!");
        break;
    case SIMD_FLOATING_POINT_EXCEPTION:
        printf("simd_floating_point_exception!");
        break;
    default:

        break;
    }
    while(1);
}
