#define ASM
.text
.global	handlers

#hanlders is a array that stores function address for each handler
handlers:
   .long divide_error,debug_exception,nmi_interrupt,breakpoint,overflow,bound_range_exceeded
   .long invalid_opcode,device_not_available,double_fault,coprocessor_segment_overrun,invalid_tss
   .long segment_not_present,stack_segment_fault,general_protection,page_fault,intel_reserved
   .long floating_point_error,alignment_check,machine_check,simd_floating_point_exception
   .long keyboard,real_time_clock,syscall_linkage,pit
syscall:
    .long 0,halt,execute,read,write,open
    .long close,getargs,vidmap,set_handler,sigreturn,ioctl,malloc,free
#the commom assembly linkage for exception/interrupt
ex_ir_linkage:
    
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

    #execute the exception handler
    movl 44(%esp),%esi  #44 can be used to locate the vector number
    cmpl $0x21,%esi
    jne pit_h
    call keyboard_handler
    jmp exc_ir_tear_down
pit_h:
    cmpl $0x20,%esi
    jne rtc
    call pit_handler
    jmp exc_ir_tear_down
rtc:
    cmpl $0x28,%esi
    jne exception
    call rtc_handler
    jmp exc_ir_tear_down

exception:
    pushl %esi #the vector number
    call exc_ir_handler
    popl %esi
    
exc_ir_tear_down:
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
    iret

#the commom assembly linkage for all handlers
syscall_linkage:
    
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

    #execute the system call
    cmpl $13,%eax #sanity check
    ja non_exist
    cmpl $1,%eax
    jb non_exist

    pushl %edx
    pushl %ecx
    pushl %ebx
    call *syscall(,%eax,4)
    popl %ebx
    popl %ecx
    popl %edx

    jmp tear_down
non_exist:
    movl $-1,%eax  #return -1 when the system call fail
tear_down:

    #restore all registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    addl $4,%esp
    popl %ds
    popl %es
    popl %fs

    iret


#all handlers are defined below
divide_error:
    pushl $0        #0 is the vector number for this kind of exception
    jmp ex_ir_linkage
debug_exception:
    pushl $1        #1 is the vector number for this kind of exception
    jmp ex_ir_linkage
nmi_interrupt:
    pushl $2        #2 is the vector number for this kind of exception
    jmp ex_ir_linkage
breakpoint:
    pushl $3        #3 is the vector number for this kind of exception
    jmp ex_ir_linkage
overflow:
    pushl $4        #4 is the vector number for this kind of exception
    jmp ex_ir_linkage
bound_range_exceeded:
    pushl $5        #5 is the vector number for this kind of exception
    jmp ex_ir_linkage
invalid_opcode:
    pushl $6        #6 is the vector number for this kind of exception
    jmp ex_ir_linkage
device_not_available:
    pushl $7        #7 is the vector number for this kind of exception
    jmp ex_ir_linkage
double_fault:
    pushl $8        #8 is the vector number for this kind of exception
    jmp ex_ir_linkage
coprocessor_segment_overrun:
    pushl $9        #9 is the vector number for this kind of exception
    jmp ex_ir_linkage
invalid_tss:
    pushl $10       #10 is the vector number for this kind of exception
    jmp ex_ir_linkage
segment_not_present:
    pushl $11       #11 is the vector number for this kind of exception
    jmp ex_ir_linkage
stack_segment_fault:
    pushl $12       #12 is the vector number for this kind of exception
    jmp ex_ir_linkage
general_protection:
    pushl $13       #13 is the vector number for this kind of exception
    jmp ex_ir_linkage
page_fault:
    pushl $14       #14 is the vector number for this kind of exception
    jmp ex_ir_linkage
intel_reserved:
    pushl $15       #15 is the vector number for this kind of exception
    jmp ex_ir_linkage
floating_point_error:
    pushl $16       #16 is the vector number for this kind of exception
    jmp ex_ir_linkage
alignment_check:
    pushl $17       #17 is the vector number for this kind of exception
    jmp ex_ir_linkage
machine_check:
    pushl $18       #18 is the vector number for this kind of exception
    jmp ex_ir_linkage
simd_floating_point_exception:
    pushl $19       #19 is the vector number for this kind of exception
    jmp ex_ir_linkage
keyboard:
    pushl $0x21     #0x21 is the vector number for keyboard interrupt
    jmp	ex_ir_linkage
real_time_clock:
    pushl $0x28     #0x28 is the vector number for RTC interrupt
    jmp ex_ir_linkage
pit:
    pushl $0x20     #0x20 is the vector number for RTC interrupt
    jmp ex_ir_linkage


