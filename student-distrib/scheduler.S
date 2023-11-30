.data
.text
    not_init: .long -2
    not_use: .long -1
    shell: .string "shell"
    ss0: .long 8
    esp0: .long 4
    kernel_ds: .word 0x18
    kernel_bottom: .long 0x800000
    task_stack_size: .long 0x2000


.global sche_index
.global	sche_array
.align 4
sche_index:
    .long 0     #the index of the scheduler array
cur_pid:
    .long -2
ebp_array:
    .long 0
    .long 0
    .long 0
sche_array:
    .long -2
    .long -2
    .long -2
.text
.global	schedule
schedule:

    #store the necessary stack information（ebp)
    movl sche_index,%eax
    movl %ebp,ebp_array(,%eax,4)

    #get the next task information
    movl sche_index,%eax
    incl %eax

index_loop: #check whether the pid>2
    cmpl $3,%eax
    jl check_pid
    addl $-3,%eax
    jmp index_loop

check_pid:

    movl %eax, sche_index

    movl sche_array(,%eax,4),%eax  #the pid for next process
    movl %eax, cur_pid
    movl not_init,%ebx
    cmpl %ebx,%eax


    je open_shell
    jmp normal_context_change

open_shell:
    
    #change the video memory map
    pushl sche_index
    call update_vidmap 
    addl $4,%esp

    #open shell
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
    addl $4,%esp

    #change tss
    movl ss0,%eax         #change ss0
    movw kernel_ds,%bx
    movw %bx,tss(%eax)
    movl kernel_bottom,%eax
    movl task_stack_size,%ebx
    imull cur_pid,%ebx
    subl %ebx,%eax
    subl $4,%eax
    movl esp0,%ebx
    movl %eax,tss(%ebx)

    #go to next program
    movl sche_index,%eax
    movl ebp_array(,%eax,4),%ebp
    leave
    ret
