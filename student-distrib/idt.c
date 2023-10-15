#include "idt.h"
#include "x86_desc.h"

void idt_init()
{

    int i = 0; // the iterator
    for (i = 0; i < 256; i++) //there is total 256 vectors in the IDT
    {
        /*set all values to zero in the entry*/
        idt[i].val[0] = 0;
        idt[i].val[1] = 0;

        /*the case when it is a exception*/
        if (i <= 0x13) //only first 0x13 exceptions are used
        {
            idt[i].dpl = 0;  //set the DPL
            set_idt_entry(idt, i);//set other values
        }

        /*the case when it is a interrupt*/
        else if (i >= 0x20 && i <= 0x2F)  //interrupts are from 0x20 to 0x2F
        {
            idt[i].dpl = 0;//set the DPL
            set_idt_entry(idt, i);//set other values
        }

        /*the case when it is a system call*/
        else if(i==0x80){      //0x80 is the vector for system call
            idt[i].dpl = 3;//set the DPL
            set_idt_entry(idt, i);//set other values
        }
    }
    lidt(idt_desc_ptr); //load the IDT
}

void set_idt_entry(idt_desc_t idt[], int index)
{
    int handler_number=index;
    if(index>=0x20 && index<=0x2F){
        handler_number=handler_number-0x20+0x14;
    }else if(index==0x80){
        handler_number=0x16;
    }
    idt[index].present = 1;  //the flag that indicates the handler exists
    idt[index].seg_selector = KERNEL_CS;//the code is in kernal space
    idt[index].size = 1;    
    idt[index].reserved1 = 1;
    idt[index].reserved2 = 1;
    idt[index].reserved3 = 1;
    SET_IDT_ENTRY(idt[index], handlers[handler_number]); //set the offset for the entry
}

void exe_exception(enum idt_type type)
{

    cli();
    /*for different types of interrupt/exception/system call, print different value*/
    switch (type)
    {

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
    case KEYBOARD:
        printf("keyboard!");
        break;
    case REAL_TIME_CLOCK:
        printf("real_time_clock!");
        break;
    case SYSTEM_CALL:
        printf("system call!");
        break;
    default:

        break;
    }
    /*hold on the screen(blue screen)*/
    while (1)
        ;
}
