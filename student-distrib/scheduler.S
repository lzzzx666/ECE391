.data
.text
.global active_terminal #it counts the number of active terminals
.global sche_index  #it records the active program index
.global	sche_array  #the array of the scheduler program pids

    not_init: .long -2      #the state when the terminal is not init
    not_active: .long -1    #the state when the terminal shut down
    shell: .string "shell"  
    ss0: .long 8            #the offset of ss0 in tss
    esp0: .long 4           #the offset of esp0 in tss
    kernel_ds: .word 0x18   #KERNEL_DS
    kernel_bottom: .long 0x800000   #the address of the bottom of the kernel
    task_stack_size: .long 0x2000   #the kernal data size for each program
    active_terminal: .long 0        #it counts the number of active terminals

.align 4
sche_index:     #it records the active program index
    .long 0     #initialize as 0(from terminal 0 to circulate)
cur_pid:        #it records the current(active) pid
    .long -2    #-2 means not init
ebp_array:      #the ebp array used for schduler
    .long 0     #initialize as 0(not important)
    .long 0
    .long 0
sche_array:     #the array of the scheduler program pids
    .long -2    #-2 means not init
    .long -2
    .long -2
    
.text
.global	schedule

/**
 * schedule
 * it is triggered by pit interrupt, and it suspends the current program then execute the 
 * program in the next terminal
 * input:none
 * output:none
 */

schedule:

    #store the necessary stack information（ebp)
    movl sche_index,%eax
    movl %ebp,ebp_array(,%eax,4)

    #increase_sche_index
    movl sche_index,%eax
    incl %eax

index_loop: #check whether the terminal index>2(get sche_index % 3)(we only have 3 terminals)
    cmpl $3,%eax
    jl check_index
    addl $-3,%eax
    jmp index_loop

check_index:
    #get the pid for next process
    movl %eax, sche_index
    movl sche_array(,%eax,4),%eax  
    movl %eax, cur_pid
    movl not_init,%ebx
    cmpl %ebx,%eax

    #the case when the shell is not init in that terminal
    je open_shell
    # movl not_active,%ebx   #now the terminal don't have inactive status
    # cmpl %ebx,%eax
    # je increase_sche_index

    #the case when a program is runnign in that terminal
    jmp normal_context_change

open_shell:
    
    #change the video memory map
    pushl sche_index
    call update_vidmap 
    addl $4,%esp #pop sche_index

    #open shell(execute)
    movl active_terminal,%eax
    incl %eax
    movl %eax,active_terminal
    movl $shell,%eax
    pushl %eax
    call execute
    popl %eax

normal_context_change:
    movl cur_pid,%eax

    #change paging for the next program
    pushl %eax
    call set_paging          
    popl %eax


    #change the video memory map
    pushl sche_index
    call update_vidmap 
    addl $4,%esp    #pop sche_index

    #change tss
    movl ss0,%eax         #change ss0
    movw kernel_ds,%bx
    movw %bx,tss(%eax)
    movl kernel_bottom,%eax
    movl task_stack_size,%ebx
    imull cur_pid,%ebx
    subl %ebx,%eax
    subl $4,%eax
    movl esp0,%ebx      #change esp0
    movl %eax,tss(%ebx)

    #go to next program
    movl sche_index,%eax
    movl ebp_array(,%eax,4),%ebp
    leave
    ret
