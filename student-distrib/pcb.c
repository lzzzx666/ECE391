#include "pcb.h"
pcb_t* pcb_array[MAX_TASK];
int8_t pcb_bitmap=0x00;
int32_t current_pid;
int32_t create_pcb(){
    int32_t pcb_index=0;
    pcb_t* new_pcb=NULL;
    int32_t i;
    while(1){
        if((pcb_bitmap>>(7-pcb_index))&0x1==TASK_NONEXIST || pcb_index>=MAX_TASK){
            break;
        }
        pcb_index++;
    }
    if(pcb_index>=MAX_TASK){
        return -1;
    }
    /*initialize the new task*/
    new_pcb= KERNAL_BOTTOM-TASK_STACK_SIZE*(pcb_index+1);
    new_pcb->pid=pcb_index;
    new_pcb->f_number=0;
    if(pcb_array[current_pid]){
        get_current_task(&current_pid);
        new_pcb->parent_pid=current_pid;
        asm("movl %%eax, %0" : "=r" (registerValue));??????????????????
    }else{
        new_pcb->parent_pid=NULL;
        new_pcb->parent_esp=NULL;
    }
    /*initialize the file table for the task*/
    for(i=0;i<MAX_FD;i++){
        new_pcb->file_obj_table[i].exist=0;
    }
    /*initialize the stdin and stdout*/
    new_pcb->file_obj_table[0].exist=1;
    new_pcb->file_obj_table[1].exist=1;
    new_pcb->file_obj_table[0].f_operation=terminal_o;
    new_pcb->file_obj_table[1].f_operation=terminal_o;
    new_pcb->file_obj_table[0].f_position=0;
    new_pcb->file_obj_table[1].f_position=0;
    new_pcb->file_obj_table[0].inode=0;
    new_pcb->file_obj_table[1].inode=0;


    pcb_array[pcb_index]=new_pcb;
    pcb_bitmap=pcb_bitmap|(0x1<<(7-pcb_index));
    return pcb_index;
}
int32_t delete_pcb(){
    pcb_bitmap=pcb_bitmap&~( 0x1<<(7-current_pid));
    return 0;
}
void get_current_task(int32_t* cur_pid){
    int32_t temp_pid;
    asm volatile(
        "movl %%esp,%%eax\n\t"
        "andl $0xffffe000,%eax\n\t"
        "movl %%eax,%0"
        :"=r"(temp_pid)::
    );
    *cur_pid=temp_pid;
}