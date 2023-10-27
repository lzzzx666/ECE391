#include "idt.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
/**
 * idt_init
 * initialize the interrupt descriptor table
 * INPUT: none
 * OUTPUT: it will set used idt entries to the specific value
 */
void idt_init()
{

    int i = 0;                // the iterator
    for (i = 0; i < IDT_NUMBER; i++) // 
    {
        /*set all values to zero in the entry*/
        idt[i].val[0] = 0;
        idt[i].val[1] = 0;

        /*the case when it is a exception*/
        if (i <= MAX_EXCEPTION) // 
        {
            idt[i].reserved3 = 1;            //the trap gate
            idt[i].dpl = DPL_0;              // set the DPL
            set_idt_entry(idt, i);           // set other values
        }

        /*the case when it is a interrupt*/
        else if (i >= INT_START && i <= INT_END) 
        {
            idt[i].reserved3 = 0;            //the int gate
            idt[i].dpl = DPL_0;              // set the DPL
            set_idt_entry(idt, i);           // set other values
        }

        /*the case when it is a system call*/
        else if (i == SYS_CALL_VEC)
        {              
            idt[i].reserved3 = 1;            //the trap gate                   
            idt[i].dpl = DPL_3;            // set the DPL
            set_idt_entry(idt, i);         // set other values
        }
    }
    lidt(idt_desc_ptr); // load the IDT
}
/**
 * set_idt_entry
 * the helper function to set the idt entry
 * INPUT: idt:the interrupt descriptor table
 *        index:the vecor number
 * OUTPUT: it will set pne idt entrie to the specific value
 */
void set_idt_entry(idt_desc_t idt[], int index)
{
    int handler_number = index;         // the index in the handler array
    if (index >= INT_START && index <= INT_END) // the case when it is a interrupt
    {
        /*two kinds of interrupt, keyboard and RTC*/
        if (index == INT_KEYBOARD)    
            handler_number = 0x14; // 0x14 is the corresponding index in the handler array
        else if (index == INT_RTC)  
            handler_number = 0x15; // 0x15 is the corresponding index in the handler array
    }
    else if (index == SYS_CALL_VEC) // the case when it is a system call
    {
        handler_number = 0x16; // 0x16 is the corresponding index in the handler array
    }
    idt[index].seg_selector = KERNEL_CS; // the code is in kernal space
    idt[index].present = PRESENT; // the flag that indicates the handler exists
    idt[index].size = 1;         //those 1s are first 3 bits of the gate value
    idt[index].reserved1 = 1;
    idt[index].reserved2 = 1;

    SET_IDT_ENTRY(idt[index], handlers[handler_number]); // set the offset for the entry
}
/**
 * exe_handler
 * it will execute different handlers according the vector value
 * INPUT: type:the vector number
 * OUTPUT: it will print the exception type of execute interrupt function
 */
void exe_handler(enum idt_type type)
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
        keyboard_handler();
        sti();
        return;
        break;
    case REAL_TIME_CLOCK:
        rtc_handler();
        sti();
        return;
    default:

        break;
    }
    /*hold on the screen(blue screen)*/
    while (1)
        ;
    sti();
}
