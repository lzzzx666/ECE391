#include "idt.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
#include "page.h"
#include "pit.h"
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
        else if(index==INT_PIT)
            handler_number= 0X17;// 0x17 is the corresponding index in the handler array
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
void exc_ir_handler(enum idt_type type)
{
    
    cli();
    int i;
    pcb_t *cur_pcb = get_current_pcb();
    /*for different types of interrupt/exception/system call, print different value*/
    switch (type)
    {

    case DIVIDE_ERROR:
        printf("divided_error!\n");
        break;
    case DEBUG_EXCEPTION:
        printf("debug_exception!\n");
        break;
    case NMI_INTERRUPT:
        printf("nmi_interrupt!\n");
        break;
    case BREAKPOINT:
        printf("breakpoint!\n");
        break;
    case OVERFLOW:
        printf("overflow!\n");
        break;
    case BOUND_RANGE_EXCEEDED:
        printf("bound_range_exceeded!\n");
        break;
    case INVALID_OPCODE:
        printf("invalid_opcode!\n");
        break;
    case DEVICE_NOT_AVAILABLE:
        printf("device_not_available!\n");
        break;
    case DOUBLE_FAULT:
        printf("double_fault!\n");
        break;
    case COPROCESSOR_SEGMENT_OVERRUN:
        printf("coprocessor_segment_overrun!\n");
        break;
    case INVALID_TSS:
        printf("invalid_tss!\n");
        break;
    case SEGMENT_NOT_PRESENT:
        printf("segment_not_present!\n");
        break;
    case STACK_SEGMENT_FAULT:
        printf("stack_segment_fault!\n");
        break;
    case GENERAL_PROTECTION:
        printf("general_protection!\n");
        break;
    case PAGE_FAULT:
        printf("page_fault!\n");
        break;
    case INTEL_RESERVED:
        printf("intel_reserved!\n");
        break;
    case FLOATING_POINT_ERROR:
        printf("floating_point_error!\n");
        break;
    case ALIGNMENT_CHECK:
        printf("alignment_check!\n");
        break;
    case MACHINE_CHECK:
        printf("machine_check!\n");
        break;
    case SIMD_FLOATING_POINT_EXCEPTION:
        printf("simd_floating_point_exception!\n");
        break;
    default:
        break;
    }
    // while(1);
    /*then go to the shell*/
    if(cur_pcb->pid==0){
        return;
    }
    /* Close all file descriptors */
    for (i = 0; i < MAX_FILE_NUM; i++)
    {   
        if (cur_pcb->file_obj_table[i].exist)
        { 
            (cur_pcb->file_obj_table[i].f_operation.close)(i);
            cur_pcb->file_obj_table[i].exist = 0;
        }
    }
    // /* Set paging for parent process */

    set_paging(cur_pcb->parent_pid);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNAL_BOTTOM - cur_pcb->parent_pid * TASK_STACK_SIZE - 4;
    delete_pcb();
    retVal=256;
    sti();
    asm volatile("movl %0, %%ebp \n\t"
                 "movl %1, %%esp \n\t"
                 "leave          \n\t"
                 "ret"
                 : /* no output */
                 :"r"(cur_pcb->parent_ebp),
                 "r"(cur_pcb->parent_esp)
                 :"ebp", "esp");
    return;
}
