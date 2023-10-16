#define ASM
.text
.global	handlers

#hanlders is a array that stores function address for each handler
handlers:
   .long divide_error,debug_exception,nmi_interrupt,breakpoint,overflow,bound_range_exceeded
   .long invalid_opcode,device_not_available,double_fault,coprocessor_segment_overrun,invalid_tss
   .long segment_not_present,stack_segment_fault,general_protection,page_fault,intel_reserved
   .long floating_point_error,alignment_check,machine_check,simd_floating_point_exception
   .long keyboard,real_time_clock,system_call
syscall:
    .long sys_halt,sys_execute,sys_read,sys_write,sys_open
    .long sys_close,sys_getargs,sys_vidmap,sys_set_handler,sys_sigreturn
#the commom assembly linkage for all handlers
common_linkage:
    
    #save all registers
    pushl %fs
    pushl %es
    pushl %ds
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushfl

    #execute the specific handler
    movl 44(%esp),%esi  #44 can be used to locate the vector number
    cmpl $0x80,%esi
    je syscall_linkage

    #the case when it is a exception or interrupt
    pushl %esi #the vector number
    call exe_handler
    popl %esi
    jmp tear_down

syscall_linkage:
    cmpl $10,%eax #10 is the maximum system call number
    ja tear_down
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *syscall(,%eax,4)
    popl %ebx
    popl %ecx
    popl %edx

tear_down:
    #restore all registers
    popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    popl %ds
    popl %es
    popl %fs

    addl $4,%esp #pop the vector number
    jmp done


#all handlers are defined below
divide_error:
    pushl $0        #0 is the vector number for this kind of exception
    jmp common_linkage
debug_exception:
    pushl $1        #1 is the vector number for this kind of exception
    jmp common_linkage
nmi_interrupt:
    pushl $2        #2 is the vector number for this kind of exception
    jmp common_linkage
breakpoint:
    pushl $3        #3 is the vector number for this kind of exception
    jmp common_linkage
overflow:
    pushl $4        #4 is the vector number for this kind of exception
    jmp common_linkage
bound_range_exceeded:
    pushl $5        #5 is the vector number for this kind of exception
    jmp common_linkage
invalid_opcode:
    pushl $6        #6 is the vector number for this kind of exception
    jmp common_linkage
device_not_available:
    pushl $7        #7 is the vector number for this kind of exception
    jmp common_linkage
double_fault:
    pushl $8        #8 is the vector number for this kind of exception
    jmp common_linkage
coprocessor_segment_overrun:
    pushl $9        #9 is the vector number for this kind of exception
    jmp common_linkage
invalid_tss:
    pushl $10       #10 is the vector number for this kind of exception
    jmp common_linkage
segment_not_present:
    pushl $11       #11 is the vector number for this kind of exception
    jmp common_linkage
stack_segment_fault:
    pushl $12       #12 is the vector number for this kind of exception
    jmp common_linkage
general_protection:
    pushl $13       #13 is the vector number for this kind of exception
    jmp common_linkage
page_fault:
    pushl $14       #14 is the vector number for this kind of exception
    jmp common_linkage
intel_reserved:
    pushl $15       #15 is the vector number for this kind of exception
    jmp common_linkage
floating_point_error:
    pushl $16       #16 is the vector number for this kind of exception
    jmp common_linkage
alignment_check:
    pushl $17       #17 is the vector number for this kind of exception
    jmp common_linkage
machine_check:
    pushl $18       #18 is the vector number for this kind of exception
    jmp common_linkage
simd_floating_point_exception:
    pushl $19       #19 is the vector number for this kind of exception
    jmp common_linkage
keyboard:
    pushl $0x21     #0x21 is the vector number for keyboard interrupt
    jmp	common_linkage
real_time_clock:
    pushl $0x28     #0x28 is the vector number for RTC interrupt
    jmp common_linkage
system_call:
    pushl $0x80     #0x80 is the vector number for system call
    jmp common_linkage
done:
    iret



